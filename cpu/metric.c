#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#include "metric.h"
#include "tool.h"

/**
 * helper function, 0 input arg
 */
__attribute__((noinline)) void foo0() { return; }

/**
 * helper function, 1 input arg
 */
__attribute__((noinline)) void foo1(int a0) { return; }

/**
 * helper function, 2 input args
 */
__attribute__((noinline)) void foo2(int a0, int a1)  { return; }

/**
 * helper function, 3 input args
 */
__attribute__((noinline)) void foo3(int a0, int a1, int a2)  { return; }

/**
 * helper function, 4 input args
 */
__attribute__((noinline)) void foo4(int a0, int a1, int a2, int a3)  { return; }

/**
 * helper function, 5 input args
 */
__attribute__((noinline)) void foo5(int a0, int a1, int a2, int a3, int a4)  { return; }

/**
 * helper function, 6 input args
 */
__attribute__((noinline)) void foo6(int a0, int a1, int a2, int a3, int a4, int a5)  { return; }

/**
 * helper function, 7 input args
 */
__attribute__((noinline)) void foo7(int a0, int a1, int a2, int a3, int a4, int a5, int a6)  { return; }

/**
 * helper function, passed to new process
 */
void * foo_target(void * rtn) {
    *((uint64_t *) rtn) = rdtsc();

    pthread_exit(NULL);
}

/**
 * Profile function call overhead
 * @param samples number of running samples to average
 */
void loops_overhead(int samples)
{
    uint64_t t0, t1;
    t0 = rdtsc();
    for (int i = 0; i < samples; ++i);
    t1 = rdtsc();
    printf("Loops for %d: %llu\n", samples, t1 - t0);
}

/**
 * Profile function call overhead
 * @param samples number of running samples to average
 */
void funccall_overhead(int samples)
{
    uint64_t t0, t1;

    t0 = rdtsc();
    for (int i = 0; i < samples; ++i)
        foo0();
    t1 = rdtsc();
    printf("Call (0 params): %.2f\n", (double)(t1 - t0) / (double)samples);

    t0 = rdtsc();
    for (int i = 0; i < samples; ++i)
        foo1(i);
    t1 = rdtsc();
    printf("Call (1 params): %.2f\n", (double)(t1 - t0) / (double)samples);

    t0 = rdtsc();
    for (int i = 0; i < samples; ++i)
        foo2(i, i);
    t1 = rdtsc();
    printf("Call (2 params): %.2f\n", (double)(t1 - t0) / (double)samples);

    t0 = rdtsc();
    for (int i = 0; i < samples; ++i)
        foo3(i, i, i);
    t1 = rdtsc();
    printf("Call (3 params): %.2f\n", (double)(t1 - t0) / (double)samples);

    t0 = rdtsc();
    for (int i = 0; i < samples; ++i)
        foo4(i, i, i, i);
    t1 = rdtsc();
    printf("Call (4 params): %.2f\n", (double)(t1 - t0) / (double)samples);

    t0 = rdtsc();
    for (int i = 0; i < samples; ++i)
        foo5(i, i, i, i, i);
    t1 = rdtsc();
    printf("Call (5 params): %.2f\n", (double)(t1 - t0) / (double)samples);

    t0 = rdtsc();
    for (int i = 0; i < samples; ++i)
        foo6(i, i, i, i, i, i);
    t1 = rdtsc();
    printf("Call (6 params): %.2f\n", (double)(t1 - t0) / (double)samples);

    t0 = rdtsc();
    for (int i = 0; i < samples; ++i)
        foo7(i, i, i, i, i, i, i);
    t1 = rdtsc();
    printf("Call (7 params): %.2f\n", (double)(t1 - t0) / (double)samples);
}

/**
 * Profile syscall overhead
 * @param samples number of running samples to average
 */
void syscall_overhead(int samples)
{
    uint64_t t0 = rdtsc();
    for (int i = 0; i < samples; ++i)
        getpid();
    uint64_t t1 = rdtsc();
    printf("Syscall: %.2f\n", (double)(t1 - t0) / (double)samples);
}

/**
 * Profile user process creation overhead
 * @param samples number of running samples to average
 */
void uproc_overhead(int samples)
{
    uint64_t t0;
    uint64_t t1;
    uint64_t sum = 0;

    // shared memory to store the creation return time of child process
    void * shmem = mmap(NULL, sizeof(uint64_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    memset(shmem, 0, sizeof(uint64_t));

    for(int i = 0; i < samples; ++i)
    {
        t0 = rdtsc();
        if(fork() == 0)
        {
            t1 = rdtsc();
            memcpy(shmem, (void *) &t1, sizeof(uint64_t));
            exit(0);
        }
        else
        {
            wait(NULL);
            sum += *((uint64_t *) shmem) - t0;
        }
    }
    printf("Fork process: %.2f\n", (double) sum / (double) samples);
}

/**
 * Profile kernel process creation overhead
 * @param samples number of running samples to average
 */
void kproc_overhead(int samples)
{
    pthread_t th;
    uint64_t t0;
    uint64_t t1;
    uint64_t sum = 0;
    for(int i = 0; i < samples; ++i)
    {
        t0 = rdtsc();
        pthread_create(&th, NULL, foo_target, (void *) &t1);
        pthread_join(th, NULL);
        sum += t1 - t0;
    }
    printf("Kernel process creation: %.2f\n", (double)sum / (double)samples);
}

/**
 * Profile process switching overhead
 * @param samples number of running samples to average
 */
void procsw_overhead(int samples)
{
    // Use blocking pipe to profile context switch time
    int * fd = malloc(2*sizeof(int));
    pipe(fd);

    uint64_t t0;
    uint64_t t1;
    uint64_t sum = 0;

    for(int i = 0; i < samples; ++i)
    {
        t0 = rdtsc();
        if(fork() == 0)
        {
            t1 = rdtsc();
            write(*(fd + 1), (void*) &t1, sizeof(uint64_t));
            exit(0);
        }
        else
        {
            t0 = rdtsc();
            read(*fd, (void*) &t1, sizeof(uint64_t));
            sum += t1 - t0;
        }
    }
    printf("Context switch process: %.2f\n", (double) sum / (double) samples);
}

