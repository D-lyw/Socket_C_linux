#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
extern int errno;

#define MAXSIZE 1024
#define SERVER_PORT 8888

int main(int argc, char const *argv[])
{
    char buf[MAXSIZE];
    socklen_t addr_len;
    int recv_l, sockfd;
    ssize_t sendLen;
    struct sockaddr_in addr, recvaddr;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(errno);
    }

    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(SERVER_PORT);

    while(1){
        fgets(buf, MAXSIZE, stdin);
        if( (sendLen = sendto(sockfd, buf, MAXSIZE, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr))) == -1){
            fprintf(stderr, "%s\n", strerror(errno));
        }
        // 清空缓存区
        bzero(buf, MAXSIZE);
        // memset(recvaddr, 0, sizeof(recvaddr));
        recv_l = recvfrom(sockfd, buf, MAXSIZE, 0, (struct sockaddr *)&recvaddr, &addr_len);
        printf("length: %d\n", recv_l);
        buf[recv_l] = '\0';
        fprintf(stdout, "Recive msg: \n %s\n", buf);
    }

    return 0;
}
