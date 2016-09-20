#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>

int main(int argc, char *argv[]) {
  mqd_t mqd = NULL;
  char* sndstr = "say hello";

  mqd = mq_open("/mq_test_queue", O_WRONLY);

  if (mqd == (mqd_t) -1)
    perror("c: mq_open\n");

  if (mq_send(mqd, "say hello", strlen(sndstr), 0) == -1)
    perror("c: mq_send\n");

  if (mq_send(mqd, "ende", strlen(sndstr), 0) == -1)
    perror("c: mq_send\n");

  exit(EXIT_SUCCESS);
}
