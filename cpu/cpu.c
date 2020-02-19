#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "tool.h"
#include "metric.h"

#define N_LOOPS_LG 1000000
#define N_LOOPS_SM 1

int main(int argc, const char * argv[])
{
    // warmup tasks
    printf("Preparing auxiliary variables...\n");
    uint64_t t1s = t1s_rdtsc();
    uint64_t toh = t_rdtsc();
#ifdef __APPLE__
    printf("rdtsc overhead cycles: %llu\n1s cycles: %llu\n", toh, t1s);
#else
    printf("rdtsc overhead cycles: %lu\n1s cycles: %lu\n", toh, t1s);
#endif
    loops_overhead(N_LOOPS_LG);
    funccall_overhead(N_LOOPS_LG);
    syscall_overhead(N_LOOPS_SM);
    uproc_overhead(N_LOOPS_SM);
    kproc_overhead(N_LOOPS_SM);
    procsw_overhead(N_LOOPS_SM);
    kprocsw_overhead(N_LOOPS_SM);

    return 0;
}
