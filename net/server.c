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
#define RECV_BUFF_SIZE 256

int main(){
    const int recv_buf_bytes = 256 * sizeof(int); // large packet
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // use IPv4
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //localhost
    serv_addr.sin_port = htons(PORT);

    // bind server address to socket
    bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    int * buf = (int *) malloc(recv_buf_bytes);

    listen(serv_sock, 20);
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    printf("Start listening on %s:%d...\n", "127.0.0.1", PORT);

    // Always waiting for new connection
    while (1) {
        int clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
        printf("Established a new connection. Start receiving\n");
        int received;
        // Always ready for receiving packets
        while (1) {
            while ((received = recv(clnt_sock, buf, recv_buf_bytes, 0)) > 0);
            if (received < 0) {
                printf("Receive done. Will close this connection.\n");
                break;
            }
        }
        close(clnt_sock);
    }
    close(serv_sock);
    return 0;
}