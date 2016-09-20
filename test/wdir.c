#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void main(int argc, char const *argv[]) {

  long maxPathLength = pathconf(".", _PC_PATH_MAX);
  char* absExecPath = malloc(sizeof((size_t)maxPathLength));
  readlink("/proc/self/exe", absExecPath, maxPathLength);
  /*
   * Trim down the pathname from the tail to remove the
   * executables name itself.
   */
  int pLen = strlen(absExecPath);
  int i;
  for (i = (pLen - 1); pLen > i; i--) {
    if (absExecPath[i] == '/') {
      break;
    }
    absExecPath[i] = '\0';
  }
  printf("absExecPath: %s\nl: %i\n", absExecPath, pLen);

  printf("%s\n", argv[0]);
}
