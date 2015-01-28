#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

int main() {
    struct rlimit lim;
    getrlimit(RLIMIT_STACK, &lim);
    if (lim.rlim_max == -1) {
        printf("stack size: unlimited\n");    
    } else {
        printf("stack size: %d\n",  (int)lim.rlim_max);
    }
    getrlimit(RLIMIT_NPROC, &lim);
    printf("process limit: %d\n", (int)lim.rlim_max);
    getrlimit(RLIMIT_NOFILE, &lim);
    printf("max file descriptors: %d\n", (int)lim.rlim_max);
    return 0;
}