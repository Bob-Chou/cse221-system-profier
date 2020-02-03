#ifndef _TOOL_H
#define _TOOL_H
#include <stdint.h>

uint64_t rdtsc_start(void);
uint64_t rdtsc_end(void);
uint64_t t1s_rdtsc(void);
uint64_t t_rdtsc(void);

#endif
