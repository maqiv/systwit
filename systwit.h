#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include "lib/plugin_manager.h"

#define STAT_FILE "/proc/stat"
#define PID_FILENAME "run/systwit.pid"
#define PID_BUF_SIZE 100
#define PLUGIN_FOLDER "./plugins"
#define MQ_NAME "/mq_systwit"
#define READ_SIZE 200
#define SLEEP_CYCLE 5

#define handle_error(msg) \
  do { printf(msg); syslog(LOG_ERR, msg); exit(EXIT_FAILURE); } while(0)

typedef struct {
  u_int32_t user;         /* Time spent in user mode */
  u_int32_t nice;         /* Time spent in user mode with low priority (nice) */
  u_int32_t systm;        /* Time spent in system mode */
  u_int32_t idle;         /* Time spent in the idle task */
  u_int32_t iowait;       /* Time waiting for I/O to complete */
  u_int32_t irq;          /* Time servicing interrupts */
  u_int32_t softirq;      /* Time servicing softirqs */
  u_int32_t steal;        /* Stolen time, which is the time spent in other operating systems when running in a virtualized environment */
  u_int32_t guest;        /* Time spent running a virtual CPU for guest operating systems under the control of the Linux kernel */
  u_int32_t guest_nice;   /* Time spent running a niced guest */
} cpu_stats;

cpu_stats cur_stat, pre_stat;
int running_state = 1;

mqd_t mq_desc = NULL;
struct mq_attr mq_attr = {
  .mq_maxmsg = 10,
  .mq_msgsize = 16
};

void read_stats_file(void);
cpu_stats read_cpu_stats(char[]);
float calculate_percentage(void);
char* calculate_percentage_string(void);
void assemble_daemon(char*);
int create_pid_file(void);
void init_message_queue(void);
int read_and_send_command_to_mq(char*);
