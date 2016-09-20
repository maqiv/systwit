#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>

int main(int argc, char *argv[]) {
  mqd_t mqd = NULL;
  struct mq_attr attr = {
    .mq_maxmsg = 10,
    .mq_msgsize = 16
  };

  mode_t perms = S_IRUSR | S_IWUSR;
  void* msgBuffer;
  ssize_t numRead;
  mq_unlink("/mq_test_queue");
  mqd = mq_open("/mq_test_queue", O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &attr);

  if (mqd == (mqd_t) -1)
    perror("s: mq_open\n");

  msgBuffer = malloc(attr.mq_msgsize);

  while (1) {
    numRead = mq_receive(mqd, msgBuffer, 16, 0);

    if (numRead == -1) {
      if (errno == EAGAIN) {
        perror("nothing here to read......\n");
        sleep(4);
        continue;
      }

      perror("s: mq_receive\n");
      exit(EXIT_FAILURE);
    }

    if (strcmp(msgBuffer, "ende") == 0)
      break;

    printf("received -> %s\n", msgBuffer);
  }


  if (mq_unlink("/mq_test_queue") == -1)
    perror("s: mq_unlink\n");

  exit(EXIT_SUCCESS);
}
