#ifndef _METRIC_H
#define _METRIC_H

/**
 * Helper function for procedure call metric.
 *
 * Function with number of parameters from 0~7. With no inline to make sure program
 * calls a function really.
 */
__attribute__((noinline)) void foo0();
__attribute__((noinline)) void foo1(int);
__attribute__((noinline)) void foo2(int, int);
__attribute__((noinline)) void foo3(int, int, int);
__attribute__((noinline)) void foo4(int, int, int, int);
__attribute__((noinline)) void foo5(int, int, int, int, int);
__attribute__((noinline)) void foo6(int, int, int, int, int, int);
__attribute__((noinline)) void foo7(int, int, int, int, int, int, int);

/**
 * Helper function for process creation metric.
 */
void * foo_target(void *arg);

/**
 * Profiler functions
 */
void loops_overhead(int);
void funccall_overhead(int);
void syscall_overhead(int);
void uproc_overhead(int);
void kproc_overhead(int);
void procsw_overhead(int);
#endif
