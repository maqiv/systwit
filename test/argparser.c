#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

  int o;
  while((o = getopt(argc, argv, "a:b")) != -1) {

    switch(o) {
    
      case 'a':
        printf("a has been given with %s\n", optarg);
        break;
      
      case 'b':
        printf("b has been given\n");
        break;
      
      case '?':
        printf("argument not found\n");
        break;
      
      default:
        printf("whut the heck?\n");
        break;
    }
  }

  exit(0);
}
