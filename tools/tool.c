#include <stdint.h>
#include <unistd.h>
#include "tool.h"

/**
 * An accurate timer based on CPU cycles, using rdtsc instruction
 * @return  the number of current CPU cycle counts
 */
uint64_t rdtsc_start(void)
{
    uint32_t hi, lo;
    __asm__ __volatile__("cpuid\n\t"
                         "rdtsc\n\t"
                         "mov %%edx, %0\n\t"
                         "mov %%eax, %1\n\t"
                         : "=r" (hi), "=r" (lo)
                         :: "%rax", "%rbx", "%rcx", "%rdx");
    return ((uint64_t) hi << 32) | lo;
}

/**
 * An accurate timer based on CPU cycles, using rdtsc instruction
 * @return  the number of current CPU cycle counts
 */
uint64_t rdtsc_end(void)
{
    uint32_t hi, lo;
    __asm__ __volatile__("rdtscp\n\t"
                         "mov %%edx, %0\n\t"
                         "mov %%eax, %1\n\t"
                         "cpuid\n\t"
                         : "=r" (hi), "=r" (lo)
                         :: "%rax", "%rbx", "%rcx", "%rdx");
    return ((uint64_t) hi << 32) | lo;
}

/**
 * Test CPU cycles in 1s
 * @return  the number of CPU cycles in 1s
 */
uint64_t t1s_rdtsc(void)
{
    uint64_t t0 = rdtsc_start();
    sleep(1);
    uint64_t t1 = rdtsc_end();
    return t1 - t0 - t_rdtsc();
}

/**
 * Test CPU cycles in one rdtsc call
 * @return  the number of CPU cycles of a rdtsc call
 */
uint64_t t_rdtsc(void)
{
    uint64_t sum = 0;
    int n_test = 100000;
    for (int i = 0; i < n_test; ++i)
    {
        uint64_t t0 = rdtsc_start();
        uint64_t t1 = rdtsc_end();
        sum += t1 - t0;
    }
    return sum / n_test;
}
