#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "tool.h"

#define PORT 1234
#define SAMPLES 10

void tcp_overhead(char *, int, int);

int main(){
    tcp_overhead("127.0.0.1", PORT, SAMPLES);
    return 0;
}

/**
 * Profile the connection establishing, rtt, and breaking overhead
 *
 * Each sample will establish a TCP connection to a "echo" system,
 * send a small packet and wait for the echo, and close it. A "echo"
 * system can be deployed using
 * ```shell
 * ncat -v -l 2000 -p ${PORT} --keep-open --exec "/bin/cat"
 * ```
 * on Linux, or
 * ```shell
 * netcat -v -l 2000 -p ${PORT} --exec "/bin/cat"
 * ```
 * on MacOS.
 */
void tcp_overhead(char * ip, int port, int samples) {
    int msg_bytes = 1 * sizeof(char);
    char * msg = (char *) malloc(msg_bytes);
    char * buf = (char *) malloc(msg_bytes);
    uint64_t t_conn = 0, t_break = 0, rtt = 0;
    uint64_t t0, t1;
    int n = samples;
    while (n-- > 0)
    {
        printf("\nBegin the %d test sample, remaining %d ...\n", samples - n, n);
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET; //use IPv4
        serv_addr.sin_addr.s_addr = inet_addr(ip); //localhost
        serv_addr.sin_port = htons(port);

        /**
         * Connection establish overhead profiling
         */
        printf("Try connecting, will keep retrying if fail. Ensure to deploy the echo server on %s:%d ...\n", ip, port);
        do
        {
            sleep(1);
            t0 = rdtsc_start();
        } while (connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1);
        t1 = rdtsc_end();
#ifdef __APPLE__
        printf("Connection established: %llu CPU cycles elapsed.\n", t1 - t0);
#else
        printf("Connection established: %lu CPU cycles elapsed.\n", t1 - t0);
#endif
        t_conn += t1 - t0;

        /**
         * RTT profiling
         */
        t0 = rdtsc_start();
        send(sock, msg, msg_bytes, 0);
        recv(sock, buf, msg_bytes, 0);
        t1 = rdtsc_end();
#ifdef __APPLE__
        printf("RTT: %llu CPU cycles elapsed.\n", t1 - t0);
#else
        printf("RTT: %lu CPU cycles elapsed.\n", t1 - t0);
#endif
        rtt += t1 - t0;

        /**
         * Connection close profiling
         */
        t0 = rdtsc_start();
        close(sock);
        t1 = rdtsc_end();
#ifdef __APPLE__
        printf("TCP closed: %llu CPU cycles elapsed.\n", t1 - t0);
#else
        printf("TCP closed: %lu CPU cycles elapsed.\n", t1 - t0);
#endif
        t_break += t1 - t0;
    }

    printf("\n************************** SUMMARY **************************\n");
    printf("TCP connection: %.2f\nRTT: %.2f\nClose: %.2f\n",
            (double)t_conn/(double)samples,
            (double)rtt/(double)samples,
            (double)t_break/(double)samples);
}