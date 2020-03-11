#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include "tool.h"

#define BLOCK_SIZE 4096

void sequential_read(int n, const char ** filelist, ssize_t * filebytes, uint64_t * readtime) {
#ifdef __APPLE__
    void * buf = malloc(BLOCK_SIZE);
#else
    void * buf = memalign(BLOCK_SIZE, BLOCK_SIZE);
#endif
    printf("Start sequential reading %d file(s)\n", n);
    int n_read = 0;
    for (int i = 0; i < n; ++i) {
        printf("Reading [%d]/[%d] ...\n", i+1, n);
        const char * filename = *(filelist + i);

        // direct I/O (block file buffer cache)
#ifdef __APPLE__
        const int fd = open(filename, O_RDONLY);
        if(fcntl(fd, F_NOCACHE, 1) == -1)
		    printf("Can't disable cache\n");
#else
        const int fd = open(filename, O_RDONLY | O_DIRECT);
#endif

        if (fd < 0) {
            printf("Cannot open file %s\n", filename);
            filebytes[i] = -1;
            readtime[i] = 0;
            continue;
        }
        ssize_t bytes_read = 0;
        ssize_t r = 0;
        uint64_t t0 = rdtsc_start();
        // read file
        while ((r = read(fd, buf, BLOCK_SIZE)) == BLOCK_SIZE)
            bytes_read += r;
        uint64_t t1 = rdtsc_end();
        close(fd);

        bytes_read += r;
        filebytes[i] = bytes_read;
        readtime[i] = t1 - t0;
        ++n_read;
    }
    printf("Reading finish! Success %d, fail %d\n", n_read, n - n_read);
}

void random_read(int n, const char ** filelist, const ssize_t * filebytes, ssize_t * readbytes, uint64_t * readtime) {
#ifdef __APPLE__
    void * buf = malloc(BLOCK_SIZE);
#else
    void * buf = memalign(BLOCK_SIZE, BLOCK_SIZE);
#endif
    printf("Start random reading %d file(s)\n", n);
    int n_read = 0;
    for (int i = 0; i < n; ++i) {
        printf("Reading [%d]/[%d] ...\n", i+1, n);
        const char * filename = *(filelist + i);

        // direct I/O (block file buffer cache)
#ifdef __APPLE__
        const int fd = open(filename, O_RDONLY);
        if(fcntl(fd, F_NOCACHE, 1) == -1)
		    printf("Can't disable cache\n");
#else
        const int fd = open(filename, O_RDONLY | O_DIRECT);
#endif

        if (fd < 0) {
            printf("Cannot open file %s\n", filename);
            readbytes[i] = -1;
            readtime[i] = 0;
            continue;
        }

        ssize_t bytes_read = 0;
        ssize_t r = 0;
        uint64_t tsum = 0;

        // make sure the total bytes we read is approximate to file size
        ssize_t blks = filebytes[i] / BLOCK_SIZE;
        for(int j = 0; j < blks; ++j) {
            int k = rand() % blks;
            uint64_t t0 = rdtsc_start();
            lseek(fd, k * BLOCK_SIZE, SEEK_SET);
            r = read(fd, buf, BLOCK_SIZE);
            uint64_t t1 = rdtsc_end();
            tsum += (t1 - t0);
            bytes_read += r;
        }
        close(fd);

        readbytes[i] = bytes_read;
        readtime[i] = tsum;
        ++n_read;
    }
    printf("Reading finish! Success %d, fail %d\n", n_read, n - n_read);
}

void singlethread_read(int fid, const char * filename, ssize_t * readbytes_shm, uint64_t * readtime_shm) {
#ifdef __APPLE__
    void * buf = malloc(BLOCK_SIZE);
#else
    void * buf = memalign(BLOCK_SIZE, BLOCK_SIZE);
#endif
    ssize_t bytes_read = 0;
    ssize_t r = 0;

	// direct I/O (block file buffer cache)
#ifdef __APPLE__
    const int fd = open(filename, O_RDONLY);
    if(fcntl(fd, F_NOCACHE, 1) == -1)
	    printf("Can't disable cache\n");
#else
    const int fd = open(filename, O_RDONLY | O_DIRECT);
#endif

    uint64_t t0 = rdtsc_start();
    // read file
    while ((r = read(fd, buf, BLOCK_SIZE)) == BLOCK_SIZE)
        bytes_read += r;
    uint64_t t1 = rdtsc_end();
    close(fd);
    bytes_read += r;

    readbytes_shm[fid] = bytes_read;
    readtime_shm[fid] = t1 - t0;
}

void multithread_read(int n, const char ** filelist, ssize_t * readbytes, uint64_t * readtime) {
    pid_t wpid, pids[n];
    int status;

    void * readbytes_shm = mmap(NULL, n * sizeof(ssize_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    void * readtime_shm = mmap(NULL, n * sizeof(uint64_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    memset(readbytes_shm, 0, n * sizeof(ssize_t));
    memset(readtime_shm, -1, n * sizeof(uint64_t));
	for(int i = 0; i < n; ++i)
	{
		pids[i] = fork();
		if(pids[i] < 0)
		{
            printf("Error in forking");
			abort();
		}
		else if(pids[i] == 0)
		{
			singlethread_read(i, filelist[i], readbytes_shm, readtime_shm);
			exit(0);
		}
	}
    // wait for all children
    while((wpid = wait(&status)) > 0);
    memcpy(readbytes, readbytes_shm, n * sizeof(ssize_t));
    memcpy(readtime, readtime_shm, n * sizeof(uint64_t));
}

void summarize(const char * sumfile, int n, const char ** filelist, const ssize_t * readbytes, const uint64_t * readtime) {
    FILE* summary = fopen(sumfile, "w");
    uint64_t t1s = (t1s_rdtsc() + t1s_rdtsc() + t1s_rdtsc()) / 3;
    fprintf(summary, "file name,file size (Bytes),blocks,total time (us),per-block time (us)\n");
    printf("\n******************** SUMMARY ********************\n");
    for (int i = 0; i < n; ++i) {
        double avgtime = readtime[i]/(double)t1s*1000000;
        ssize_t blks = readbytes[i]/BLOCK_SIZE;
        fprintf(summary, "%s,%zd,%zd,%.6f,%.6f\n",
            filelist[i],
            readbytes[i],
            blks,
            avgtime,
            avgtime/blks);
        printf("file: \"%s\", size: %zd, blocks: %zd, time: %.2f us, per-block time: %.2f us\n",
            filelist[i],
            readbytes[i],
            blks,
            avgtime,
            avgtime/blks);
    }
    fclose(summary);
    printf("*************************************************\n\n");
}

int main() {
    /**
     * Large fake files
     */
    const char * filelist[] = {
        "f.4K.d",
        "f.8K.d",
        "f.16K.d",
        "f.32K.d",
        "f.64K.d",
        "f.128K.d",
        "f.256K.d",
        "f.512K.d",
        "f.1M.d",
        "f.2M.d",
        "f.4M.d",
        "f.8M.d",
        "f.16M.d",
        "f.32M.d",
        "f.64M.d",
        "f.128M.d",
        "f.256M.d",
        "f.512M.d",
        "f.1024M.d"
    };
    int n = 19;
    ssize_t * filebytes = (ssize_t *) malloc(n * sizeof(ssize_t));
    uint64_t * readtime = (uint64_t *) malloc(n * sizeof(ssize_t));
    ssize_t * readbytes = (ssize_t *) malloc(n * sizeof(ssize_t));

    /**
     * Sequential read
     */
    memset(readbytes, 0, n * sizeof(ssize_t));
    memset(filebytes, 0, n * sizeof(ssize_t));
    memset(readtime, 0, n * sizeof(ssize_t));
    sequential_read(n, filelist, filebytes, readtime);
    summarize("sequential.csv", n, filelist, filebytes, readtime);

    /**
     * Random read
     */
    memset(readbytes, 0, n * sizeof(ssize_t));
    memset(readtime, 0, n * sizeof(ssize_t));
    random_read(n, filelist, filebytes, readbytes, readtime);
    summarize("random.csv", n, filelist, readbytes, readtime);

    /**
     * Multithreading read
     */
    memset(readbytes, 0, n * sizeof(ssize_t));
    memset(readtime, 0, n * sizeof(ssize_t));
    multithread_read(n, filelist, readbytes, readtime);
    summarize("multithreading.csv", n, filelist, readbytes, readtime);

    return 0;
}