#include <stdio.h>
#include <stdlib.h>
#ifdef __Unikraft__
#include <uk/config.h>
#endif /* __Unikraft__ */
#include <uk/syscall.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <uk/uk_signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <uk/config.h>
#include <uk/alloc.h>
#include <uk/sched.h>
#include <signal.h>
#include <string.h>
#include <uk/thread.h>
#include <uk/uk_signal.h>
#include <uk/essentials.h>
#include <uk/process.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include <pthread.h>
#include <sys/prctl.h>

__thread volatile char test_var_bss[3000];
__thread volatile char test_var_data[3000] = {34,4,4,4,3,32,2,45,4,3,4,3,43};

int main() {

    printf("0x%x.0x%x.0x%x\n",
            test_var_data[0],
            test_var_data[1],
            test_var_data[2]);

    return 0;
}
