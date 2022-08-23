#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if(argc < 2) {    // error message
    fprintf(2, "Usage: sleep...\n");
    exit(1);
  }
  if(sleep(atoi(argv[1])) < 0)
    fprintf(2, "sleep: failed to sleep\n");
  exit(0);
}