#include <stdint.h>
#include <unistd.h>
#include "tool.h"

/**
 * An accurate timer based on CPU cycles, using rdtsc instruction
 * @return  the number of current CPU cycle counts
 */
uint64_t rdtsc(void)
{
    uint32_t hi, lo;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t) hi << 32) | lo;
}

/**
 * Test CPU cycles in 1s
 * @return  the number of CPU cycles in 1s
 */
uint64_t t1s_rdtsc(void)
{
    uint64_t t0 = rdtsc();
    sleep(1);
    uint64_t t1 = rdtsc();
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
        uint64_t t0 = rdtsc();
        uint64_t t1 = rdtsc();
        sum += t1 - t0;
    }
    return sum / n_test;
}
