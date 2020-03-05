#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "tool.h"

#define PORT 1234
#define N_CONNECTS 10 // only handle 1 connection

int main(){
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // use IPv4
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //localhost
    serv_addr.sin_port = htons(PORT);

    // bind server address to socket
    bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));


    int n_conn = N_CONNECTS;
    uint64_t * ts = (uint64_t *) malloc(2 * sizeof(uint64_t));
    while (n_conn-- > 0)
    {
        listen(serv_sock, 20);
        printf("Start listening ...\n");
        struct sockaddr_in clnt_addr;
        socklen_t clnt_addr_size = sizeof(clnt_addr);
        int clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
        printf("Established the %d connection, remaining %d ...\n", N_CONNECTS - n_conn, n_conn);

        ts[0] = rdtsc_end();
        // sleep to make sure read will block the client so that our timing is more accurate
        sleep(3);
        ts[1] = rdtsc_start();

        send(clnt_sock, ts, 2 * sizeof(uint64_t), 0);
        close(clnt_sock);
    }
    close(serv_sock);
    return 0;
}