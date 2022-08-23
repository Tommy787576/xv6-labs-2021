#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if(argc > 1) {    // error message
    fprintf(2, "Usage: pingpong...\n");
    exit(1);
  }
  int p[2];
  char buf[1];
  pipe(p);
  if (fork() == 0) {    // child process
    read(p[0], buf, sizeof(buf));
    fprintf(1, "%d: received ping\n", getpid());
    write(p[1], "c", 1);
  }
  else {    // parent process
    write(p[1], "p", 1);
    wait(0);
    read(p[0], buf, sizeof(buf));
    fprintf(1, "%d: received pong\n", getpid());
  }
  exit(0);
}