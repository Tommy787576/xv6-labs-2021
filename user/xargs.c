#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
  char buf[512], ch;
  char* argv2[MAXARG];
  int n;

  for (int i = 1; i < argc; i++)
    argv2[i - 1] = argv[i];
  
  while (1) { // read inputs from the standard input
        int bufIdx = 0;
        while(1) {
            n = read(0, &ch, sizeof(ch));
            if (n == 0)
                exit(0);
            if (n < 0) {
                fprintf(2, "xargs error\n");
                exit(1);
            }
            if (ch == '\n')
                break;
            buf[bufIdx++] = ch;
        }
        buf[bufIdx] = '\0';
        if (fork() == 0) {
            argv2[argc - 1] = buf;
            argv2[argc] = '\0';
            exec(argv2[0], argv2);
            fprintf(2, "xargs error\n");
            exit(1);
        }
        else 
            wait(0);
  }
}