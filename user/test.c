#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void) {
    unsigned int i = 0x00646c72;
    printf("H%x Wo%s", 57616, &i);
    printf("\nx=%d y=%d", 3, 4);
    exit(0);
}