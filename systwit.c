/*
 * A daemon that reads the /proc/stats file, calculates the cpu usage
 * percentage and writes it to ?????syslog?????.
 *
 * Copyright (C) 2015-2016 Sebastian Glinski <sebastian.glinski@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * TODO:
 * - implement using a pid-file instead of checking if
 *   there exists a message queue #DONE
 * - unload all plugins and free up space on exit
 * - cleanup code
 * - implement signal handling
 *
 * COMMANDS:
 * 'stop': stop the daemon
 * 'reload': reload plugins
 *
 */

#include "systwit.h"

int main(int argc, char *argv[]) {
  char t_perc[12];

  /* check if a PID file exists. if yes, create one. */
  if(access(PID_FILENAME, F_OK) != -1) {
    // Process is already running.....read and send commands to it!
    if(argc <= 1) {
      syslog(LOG_ERR, "Daemon already running. No command given.");
      printf("Daemon already running. No command given.\n");
      exit(EXIT_FAILURE);
    }

    if(read_and_send_command_to_mq(argv[1]) == 0) {
      exit(EXIT_SUCCESS);
    } else {
      exit(EXIT_FAILURE);
    }
  }

  create_pid_file();

  init_message_queue();

  /*assemble_daemon(argv[argv[0]]);*/

  /* load plugins */
  load_plugins(PLUGIN_FOLDER);

  /* the main loop where the daemon runs in */
  while(running_state) {
    read_stats_file();
    sprintf(t_perc, "%.7f", calculate_percentage());
    execute_plugins(t_perc);
    sleep(5);
  }

  /* clean up stuff */

  closelog();

  exit(EXIT_SUCCESS);
}

void read_stats_file(void) {
  char puffer[READ_SIZE];
  int t_idle = 0, t_iowait = 0;
  
  FILE *file = fopen(STAT_FILE, "r");

  if (file) {
    /* Read the first line of the stat file */
    fgets(puffer, READ_SIZE, file);

    /*
     * Keep the statistics from the last run
     * in pre_stat
     */
    pre_stat = cur_stat;
    cur_stat = read_cpu_stats(puffer);

    /*
    syslog(LOG_INFO, "%u - %u - %u - %u - %u - %u - %u - %u - %u - %u", pre_stat.user, pre_stat.nice, pre_stat.systm, pre_stat.idle, pre_stat.iowait, pre_stat.irq, pre_stat.softirq, pre_stat.steal, pre_stat.guest, pre_stat.guest_nice);
    syslog(LOG_INFO, "%u - %u - %u - %u - %u - %u - %u - %u - %u - %u", cur_stat.user, cur_stat.nice, cur_stat.systm, cur_stat.idle, cur_stat.iowait, cur_stat.irq, cur_stat.softirq, cur_stat.steal, cur_stat.guest, cur_stat.guest_nice);
    */
  }

  /*
  if (file) {
    fclose(file);
    file = NULL;
  }
  */
}

cpu_stats read_cpu_stats(char c[]) {
  cpu_stats s;
  /*char *tokens[15];*/
  char *t;
  int i = 0;

  /* Skip the first token which is the string 'cpu' */
  strtok(c, " ");

  /*
   * Read the tokens from the first cpu-line and parse
   * them into the structure
   */
  t = strtok(NULL, " ");
  while(t != NULL) {
    sscanf(t, "%u", &((u_int32_t*)&s)[i]);
    t = strtok(NULL, " ");
    i++;
  }

  return s;
}

float calculate_percentage(void) {
  u_int32_t prevIdle = pre_stat.idle + pre_stat.iowait;
  u_int32_t idle = cur_stat.idle + cur_stat.iowait;

  u_int32_t prevNonIdle = pre_stat.user + pre_stat.nice + pre_stat.systm + pre_stat.irq + pre_stat.softirq + pre_stat.steal;
  u_int32_t nonIdle = cur_stat.user + cur_stat.nice + cur_stat.systm + cur_stat.irq + cur_stat.softirq + cur_stat.steal;

  u_int32_t totald = (idle + nonIdle) - (prevIdle + prevNonIdle);
  u_int32_t idled = idle - prevIdle;

  /* float CPU_Percentage = (totald - idled)/(float)totald; */

  return (totald - idled) / (float)totald;
}

void assemble_daemon(char* app_name) {
  pid_t pid, sid;

  /* Fork off the parent process */
  if((pid = fork()) < 0) {
    exit(EXIT_FAILURE);
  }

  /*
   * If we got a good pid then we
   * can exit the parent process
   */
  if(pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /*
   * Change the file mode mask to 0
   * so we will have full access to the
   * files generated by the daemon.
   */
  umask(0);

  /* Connect to the syslog daemon */
  openlog(app_name, LOG_NOWAIT|LOG_PID, LOG_USER);
  syslog(LOG_NOTICE, "systwit daemon successfully startet");

  /*
   * Create a new session id for the
   * child process
   */
  sid = setsid();
  if(sid < 0) {
    syslog(LOG_ERR, "Could not create process group\n");
    exit(EXIT_FAILURE);
  }

  /*
   * Change the working directory
   * where the daemon operates in
   */
  if((chdir("/")) < 0) {
    syslog(LOG_ERR, "Could not change working directory to /\n");
    exit(EXIT_FAILURE);
  }

  /*
   * Close the standard file descriptors because we don't need
   * them anymore.
   */
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  /*signal(SIGHUP, signal_handler);
  signal(SIGKILL, signal_handler);*/
}

void signal_handler(int sig) {
  switch (sig) {
    case SIGHUP:
      syslog(LOG_INFO, "systwit - catched a SIGHUP signal");
      break;
    case SIGTERM:
      syslog(LOG_INFO, "systwit - catched a SIGTERM signal");
      break;
    default:
      syslog(LOG_INFO, "systwit - catched an unknown signal");
      break;
  }
}

int create_pid_file(void) {
  int fd;
  int flags;
  char buf[PID_BUF_SIZE];
  struct flock fl = {
    .l_type = F_WRLCK,
    .l_whence = SEEK_SET,
    .l_start = 0,
    .l_len = 0
   };

  fd = open(PID_FILENAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if(fd == -1) {
    syslog(LOG_ERR, "Could not create PID file %s", PID_FILENAME);
    exit(EXIT_FAILURE);
  }

  flags = fcntl(fd, F_GETFD); /* Fetch flags */
  if(flags == -1) {
    syslog(LOG_ERR, "Could not get flags for PID file %s", PID_FILENAME);
    exit(EXIT_FAILURE);
  }

  if(fcntl(fd, F_SETFD, flags | 1) == -1) {
    syslog(LOG_ERR, "Could not set flags for PID file %s", PID_FILENAME);
    exit(EXIT_FAILURE);
  }

  if(fcntl(fd, F_SETLK, &fl) == -1) {
    if(errno == EAGAIN || errno == EACCES) {
      syslog(LOG_ERR, "PID file '%s' is already locked; daemon is probably running....?!", PID_FILENAME);
    } else {
      syslog(LOG_ERR, "Unable to lock PID file %s", PID_FILENAME);
    }
    exit(EXIT_FAILURE);
  }

  if(ftruncate(fd, 0) == -1) {
    syslog(LOG_ERR, "Could not truncate PID file %s", PID_FILENAME);
    exit(EXIT_FAILURE);
  }

  snprintf(buf, PID_BUF_SIZE, "%ld\n", (long) getpid());
  if(write(fd, buf, strlen(buf)) != strlen(buf)) {
    syslog(LOG_ERR, "Writing to PID file '%s' failed", PID_FILENAME);
    exit(EXIT_FAILURE);
  }

  return fd;
}

int read_and_send_command_to_mq(char* arg) {

  int ret = 0;

  if(strcmp(arg, "reload") == 0 ||
     strcmp(arg, "stop") == 0) {
      printf("will parse argument %s\n", arg);
      //TODO
  } else {
      syslog(LOG_ERR, "Command %s not found.", arg);
      printf("Command %s not found.\n", arg);
      ret = -1;
  }

  return ret;

}

void init_message_queue(void) {
/*  mqd_t mqdes = NULL;
  struct mq_attr attr = {
    .mq_maxmsq = 10,
    .mq_msgsize = 16
  };*/

  mode_t perms = S_IRUSR | S_IWUSR;

  mq_unlink(MQ_NAME);
  mq_desc = mq_open(MQ_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &mq_attr);

  if(mq_desc == (mqd_t) -1) {
    syslog(LOG_ERR, "Cound not create message queue.");
    exit(EXIT_FAILURE);
  }

}

void read_message_queue(void) {
  void* mq_read_buf = malloc(16);
  ssize_t numRead = mq_receive(mq_desc, mq_read_buf, mq_attr.mq_msgsize, NULL);
  if(numRead >= 0) {
    if(strcmp(mq_read_buf, "reload") == 0) {
      //reload the plugins
    } else if(strcmp(mq_read_buf, "stop") == 0) {
      syslog(LOG_INFO, "Received '%s' command. Stopping daemon....", (char*)mq_read_buf);
      running_state = 0;
    }
  }
  free(mq_read_buf);
}
