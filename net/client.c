#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "tool.h"

#define PORT 1234
#define SAMPLES 100
#define PACKET_SIZE 100 // packet size in KB

void heavy_writer(char *, int, int);

int main(){
    heavy_writer("127.0.0.1", PORT, SAMPLES);
    return 0;
}

/**
 * Profile the peak bandwidth, will write a large packet to the server.
 *
 * Can either deploy an echo server in shell using
 * ```shell
 * ncat -v -l 2000 -p ${PORT} --keep-open --exec "/bin/cat"
 * ```
 * on Linux, or
 * ```shell
 * netcat -v -l 2000 -p ${PORT} --exec "/bin/cat"
 * ```
 * on MacOS, or launch the server defined in server.cpp
 */
void heavy_writer(char * ip, int port, int samples) {
    const int msg_bytes = 1024 * PACKET_SIZE; // large packet
    int * msg = (int *) malloc(msg_bytes);
    memset(msg, 0, msg_bytes);
    uint64_t t_peak = 0;
    uint64_t t0, t1;
    int n = samples;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //use IPv4
    serv_addr.sin_addr.s_addr = inet_addr(ip); //localhost
    serv_addr.sin_port = htons(port);
    /**
     * Connect to server
     */
    printf("Try connecting, will keep retrying if fail. Ensure to deploy the server on %s:%d ...\n", ip, port);
    do
    {
        sleep(1);
    } while (connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1);


    while (n-- > 0)
    {
        printf("Begin the %d test sample, remaining %d ...\n", samples - n, n);
        /**
         * Peak performance profiling
         */
        t0 = rdtsc_start();
        int bytes_send;
        if ((bytes_send = send(sock, msg, msg_bytes, 0)) != msg_bytes) {
            printf("Packet sent error, will retry, sent %d\n", bytes_send);
            ++n;
            continue;
        }
        t1 = rdtsc_end();
#ifdef __APPLE__
        printf("Packet sent: %llu CPU cycles elapsed.\n", t1 - t0);
#else
        printf("Packet sent: %lu CPU cycles elapsed.\n", t1 - t0);
#endif
        t_peak += t1 - t0;
    }
    close(sock);

    printf("\n************************** SUMMARY **************************\n");
    printf("Peak send cycles: %.2f, or %.2fMB/s\n",
            (double)t_peak/(double)samples,
            (PACKET_SIZE/((double)t_peak/(double)samples/(double)t1s_rdtsc())/1024));
}