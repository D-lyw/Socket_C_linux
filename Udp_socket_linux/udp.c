#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>

extern int errno;

#define MAXMSGSIZE 1024


char recvbuf[MAXMSGSIZE];
char sendbuf[MAXMSGSIZE];

// 线程描述符
pthread_t Psend;
// i套接字描述符
int sockfd;

struct sockaddr_in localaddr, destaddr;

void *sendMsg(void *s){
    int sendLength;
    while(1){
        fgets(sendbuf, MAXMSGSIZE, stdin);
        if((sendLength = sendto(sockfd, sendbuf, MAXMSGSIZE, 0, 
            (struct sockaddr *)&localaddr, sizeof(struct sockaddr)) ) == -1){
            fprintf(stderr, " %s\n", strerror(errno));
        }
    }
    return NULL;
}

int main(int argc, char const *argv[])
{
    if(argc != 2){
        fprintf(stderr, "Usage: ./udp < PORT >\n");
        exit(errno);
    }

    socklen_t addrlen;
    int n;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        fprintf(stderr, "创建套接字失败 , %s\n", strerror(errno));
        exit(errno);
    }

    bzero(&localaddr, sizeof(struct sockaddr_in));
    bzero(&destaddr, sizeof(struct sockaddr_in));

    // 本地套接字地址信息
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    localaddr.sin_port = htons(atoi(argv[1]));

    if(bind(sockfd, (struct sockaddr *)&localaddr, sizeof(struct sockaddr_in)) == -1){
        fprintf(stderr, "绑定套接字失败, %s\n", strerror(errno));
    }

    pthread_create(&Psend, NULL, sendMsg, NULL);
    // 循环阻塞接收
    while(1){
        n = recvfrom(sockfd, recvbuf, MAXMSGSIZE, 0, 
            (struct sockaddr *)&destaddr, &addrlen);
        printf("Recive from:%s:%d\n", inet_ntoa(destaddr.sin_addr), ntohs(destaddr.sin_port));
        bzero(recvbuf, MAXMSGSIZE);
    }
    
    close(sockfd);
    return 0;
}

