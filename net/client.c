#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "tool.h"

#define PORT 1234
#define N_PACKETS 10 // only handle 1 connection

int main(){
    int samples = N_PACKETS;
    int n_packets = N_PACKETS;
    uint64_t * ts = (uint64_t *) malloc(2 * sizeof(uint64_t));
    uint64_t d1 = 0, d2 = 0, d3 = 0;
    uint64_t t0, t1, t2, t3, t4;
    while (n_packets-- > 0)
    {
        printf("Begin the %d test, remaining %d ...\n", N_PACKETS - n_packets, n_packets);
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET; //use IPv4
        serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //localhost
        serv_addr.sin_port = htons(PORT);
        do
        {
            sleep(3);
            t0 = rdtsc_start();
        } while (connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1);
        printf("Connection established, begin benchmarking ...\n");

        t1 = rdtsc_start();
        recv(sock, ts, 2 * sizeof(uint64_t), 0);
        t4 = rdtsc_start();
        t2 = ts[0];
        t3 = ts[1];
        d1 += t1 - t0;
        d2 += t2 - t1;
        d3 += t4 - t3;
        close(sock);
    }
    printf("\n************************** SUMMARY **************************\n");
    printf("connection established: %.2f\none-way: %.2f\nsend + receive: %.2f\n",
            (double)d1/(double)samples,
            (double)d2/(double)samples,
            (double)d3/(double)samples);
    free(ts);
    return 0;
}
