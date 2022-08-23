#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void pipeline(int* p) {
  int prime, n, first = 1;
  int q[2];

  pipe(q);
  read(p[0], &prime, sizeof(prime));  // get a number from left neighbor 
  fprintf(1, "prime %d\n", prime);  
  while(read(p[0], &n, sizeof(n))) {  // get a number from left neighbor
    if (n % prime != 0) {
      if (first) {
        if (fork() == 0) {
          close(q[1]);  // child doesn't need write!!
          pipeline(q);
        }
        close(q[0]);    // parent doesn't need read!!
        first = 0;
      }
      write(q[1], &n, sizeof(n)); // send n to right neighbor
    }
  }
  close(q[1]);  // parent close the write side
  wait(0);
  exit(0);
}

int main(int argc, char *argv[]) {
  if(argc > 1) {    // error message
    fprintf(2, "Usage: prime...\n");
    exit(1);
  }

  int p[2];
  pipe(p);
  if (fork() == 0) {  // child process
    close(p[1]);  // child doesn't need write!!
    pipeline(p);
  }
  else {  // parent process
    close(p[0]);  // parent doesn't need read!!
    for (int i = 2; i <= 35; i++)
      write(p[1], &i, sizeof(i));
    close(p[1]);
  }
  wait(0);
  exit(0);
}