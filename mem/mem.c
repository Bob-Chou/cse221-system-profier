#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "tool.h"

#define SAMPLES 1
#define PAGE_SIZE 4096 // page size in bytes
#define _1KB 1024
#define _4KB 4*_1KB
#define _16KB 16*_1KB
#define _64KB 64*_1KB
#define _256KB 256*_1KB
#define _1MB 1024*1024
#define _16MB 16*_1MB
#define _128MB 128*_1MB
#define _256MB 256*_1MB
#define _1GB 1024*_1MB

void pagefault(const char * swapname, ssize_t filesize) {
    /**
     *  Disable the file cache for more accurate measurement!
     *      + In MacOS, we use F_NOCACHE flag
     *      + In Linux, we use O_DIRECT flag
     */
#ifdef __APPLE__
    const int fd = open(swapname, O_RDONLY);
    if(fcntl(fd, F_NOCACHE, 1) == -1)
        printf("Can't disable cache\n");
#else
    const int fd = open(filename, O_RDONLY | O_DIRECT);
#endif

    uint64_t tsum = 0;
    uint64_t t1s = (t1s_rdtsc() + t1s_rdtsc() + t1s_rdtsc()) / 3;
    ssize_t pages = filesize / PAGE_SIZE;
    printf("Begin swapping in %zd MB (%zd pages) from file %s\n", filesize/_1MB, pages, swapname);
    for (int i = 0; i < SAMPLES; ++i) {
        int f = open(swapname, O_RDWR);
        if (f < 0) {
            printf("open large file failed\n");
            return;
        }

        // mem map to file
        char* mmem =(char*) mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, f, 0);
        for (ssize_t i = pages - 1; i >= 0; --i) {
            uint64_t t0 = rdtsc_start();
            volatile char value = mmem[i * PAGE_SIZE];
            uint64_t t1 = rdtsc_end();
            tsum += t1 - t0;
        }
        munmap(mmem, filesize);
    }
#ifdef __APPLE__
    printf("\n********************* Pagefault summary *********************\nswapped pages: %zd\nper page cycles: %llu\nper page time (ms): %.6f\n", pages, tsum/pages, (double)tsum/(double)pages*1000/t1s);
#else
    printf("\n********************* Pagefault summary *********************\nswapped pages: %zd\nper page cycles: %lu\nper page time (ms): %.6f\n", pages, tsum/pages, (double)tsum/(double)pages*1000/t1s);
#endif
}

void mempeak(ssize_t arr_size, const char * sumfile) {
    uint64_t tsum = 0;
    ssize_t arrlen = arr_size / sizeof(int);
    for (int i = 0; i < SAMPLES; ++i) {
        int * src = (int *) malloc(arr_size);
        int * dest = (int *) malloc(arr_size);
        uint64_t t0 = rdtsc_start();
        memcpy(dest, src, arr_size);
        uint64_t t1 = rdtsc_end();
        tsum += t1 - t0;
        free(src);
        free(dest);
    }
    tsum /= SAMPLES;

    uint64_t t1s = (t1s_rdtsc() + t1s_rdtsc() + t1s_rdtsc()) / 3;
    FILE* summary = fopen(sumfile, "w");
    double per_access_time = (double)tsum/(double)arrlen*1000/t1s*1000;
    double peak_bw = arr_size*t1s/1024/1024/(double)tsum;

    fprintf(summary, "mem size (bytes),per access time (us),peak bandwidth (MB/s)\n");
    fprintf(summary, "%zd,%.6f,%.6f\n", arr_size, per_access_time, peak_bw);
    fclose(summary);

    printf("\n********************* Access summary *********************\n");
    printf("array size: %zd\nper access read cycles: %llu\nper access read time (us): %.6f\npeak bandwidth: %.6f MB/s\n",
        arr_size,
        tsum/arrlen,
        per_access_time,
        peak_bw);
}

void memaccess(ssize_t arr_size, ssize_t step, const char * sumfile) {
    uint64_t r_tsum = 0;
    uint64_t w_tsum = 0;
    step /= 4; // an int is 4 Bytes
    ssize_t arrlen = arr_size / sizeof(int);
    for (int i = 0; i < SAMPLES; ++i) {
        int * array = (int *) malloc(arr_size);
        for (int j = 0; j < arrlen; ++j) {
            uint64_t t0 = rdtsc_start();
            int value = *(array + (j * step) % arrlen);
            uint64_t t1 = rdtsc_end();
            r_tsum += t1 - t0;
        }
        free(array);
    }
    r_tsum /= SAMPLES;

    for (int i = 0; i < SAMPLES; ++i) {
        int * array = (int *) malloc(arr_size);
        for (int j = 0; j < arrlen; ++j) {
            uint64_t t0 = rdtsc_start();
            *(array + (j * step) % arrlen) = j;
            uint64_t t1 = rdtsc_end();
            w_tsum += t1 - t0;
        }
        free(array);
    }
    w_tsum /= SAMPLES;
    uint64_t t1s = (t1s_rdtsc() + t1s_rdtsc() + t1s_rdtsc()) / 3;
    FILE* summary = fopen(sumfile, "w");
    double per_read_time = (double)r_tsum/(double)arrlen*1000/t1s*1000;
    double per_write_time = (double)w_tsum/(double)arrlen*1000/t1s*1000;
    double read_speed = arr_size*t1s/1024/1024/(double)r_tsum;
    double write_speed = arr_size*t1s/1024/1024/(double)w_tsum;

    fprintf(summary, "mem size (bytes),step (bytes),per access read time (us),per access write time (us),read speed (MB/s),write speed (MB/s)\n");
    fprintf(summary, "%zd,%zd,%.6f,%.6f,%.6f,%.6f\n", arr_size, step, per_read_time, per_write_time, read_speed, write_speed);
    fclose(summary);

    printf("\n********************* Access summary *********************\n");
    printf("array size: %zd\naccess step: %zd\nper access read cycles: %llu\nper access read time (us): %.6f\noverall read speed: %.6f MB/s\n",
        arr_size,
        step,
        r_tsum/arrlen,
        per_read_time,
        read_speed);
    printf("\narray size: %zd\naccess step: %zd\nper access write cycles: %llu\nper access write time (us): %.6f\noverall write speed: %.6f MB/s\n",
        arr_size,
        step,
        w_tsum/arrlen,
        per_write_time,
        write_speed);
}

int main(){
    // pagefault("swap.d", _1GB);
    mempeak(_256KB, "peak.csv");
    // memaccess(_16MB, _1KB, "summary_16MB_1KB.csv");
    // memaccess(_16MB, _4KB, "summary_16MB_4KB.csv");
    // memaccess(_16MB, _16KB, "summary_16MB_16KB.csv");
    // memaccess(_16MB, _64KB, "summary_16MB_64KB.csv");
    // memaccess(_16MB, _256KB, "summary_16MB_256KB.csv");
    // memaccess(_16MB, _1MB, "summary_16MB_256KB.csv");
    return 0;
}