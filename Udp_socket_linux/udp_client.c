#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

extern int errno;

#define MAXSIZE 1024
#define SERVER_PORT 8888
char sendbuf[MAXSIZE];
char recvbuf[MAXSIZE];

    socklen_t addr_len;
    int recv_l, sockfd;
    ssize_t sendLen;
    struct sockaddr_in seraddr, recvaddr;

pthread_t Psend;

void *sendMsg(void *s){
    while(1){
        fgets(sendbuf, MAXSIZE, stdin);
        if( (sendLen = sendto(sockfd, sendbuf, MAXSIZE, 0, (struct sockaddr *)&seraddr, sizeof(struct sockaddr))) == -1){
            fprintf(stderr, "%s\n", strerror(errno));
        }
    }
    return NULL;
}

int main(int argc, char const *argv[])
{
    


    // 创建一个客户端的套接字
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(errno);
    }

    bzero(&seraddr, sizeof(struct sockaddr_in));
    // 服务器端地址信息
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    seraddr.sin_port = htons(SERVER_PORT);

    pthread_create(&Psend, NULL, sendMsg, NULL);
    
    while(1){
        // 清空缓存区
        bzero(recvbuf, MAXSIZE);
        if((recv_l = recvfrom(sockfd, recvbuf, MAXSIZE, 0, (struct sockaddr *)&recvaddr, &addr_len) == -1)){
            fprintf(stderr, "%s\n", strerror(errno));
        }
        fprintf(stdout, "Recived from:%s:%d\n#> %s", inet_ntoa(recvaddr.sin_addr), ntohs(recvaddr.sin_port), recvbuf);
    }

    return 0;
}
