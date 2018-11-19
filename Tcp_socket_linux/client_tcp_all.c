/*
 * @Author: D-lyw 
 * @Date: 2018-11-17 00:41:07 
 * @Last Modified by:   D-lyw 
 * @Last Modified time: 2018-11-17 00:41:07 
 * @Descripe Tcp客户端 连接服务端 双向通信
 */

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

#define MAXSIZE 1024*5
#define SERVER_PORT 3003
const char *LOCALIP = "127.0.0.1";

char sendbuf[MAXSIZE];
char recvbuf[MAXSIZE];

int sockfd;

void *sendMsg(void *s){
    while(1){
        printf("#> ");
        fgets(sendbuf, MAXSIZE, stdin);
        if(send(sockfd,sendbuf, MAXSIZE, 0) == -1){
            fprintf(stderr, "%s\n#> ", strerror(errno));
        }
        if(strncmp(sendbuf, "end", 3) == 0){
            close(sockfd);
            exit(0);
        }
    }
    return NULL;
}

int main(int argc, char const *argv[])
{
    ssize_t sendLen;
    struct sockaddr_in seraddr, recvaddr;
    pthread_t Psend;
    // 创建一个客户端的套接字
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(errno);
    }

    bzero(&seraddr, sizeof(struct sockaddr_in));
    // 服务器端地址信息
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = inet_addr(LOCALIP);
    seraddr.sin_port = htons(SERVER_PORT);

    // 请求连接服务器进程
    if(connect(sockfd, (struct sockaddr *)&seraddr, sizeof(struct sockaddr)) == -1){
        fprintf(stderr,"请求连接服务器失败, %s\n", strerror(errno));
        exit(errno);
    }
    
    printf("--------Successful connect to %s:%d--------\n", inet_ntoa(seraddr.sin_addr), ntohs(seraddr.sin_port));

    // 新建线程发送消息
    pthread_create(&Psend, NULL, sendMsg, NULL);
    
    while(1){
        // 清空缓存区
        bzero(recvbuf, MAXSIZE);
        if(recv(sockfd, recvbuf, MAXSIZE, 0) == -1){
            fprintf(stderr, "%s\n", strerror(errno));
        }
        fprintf(stdout, "Recived from:%s:%d\n#> %s\n", inet_ntoa(recvaddr.sin_addr), ntohs(recvaddr.sin_port), recvbuf);
    }
    return 0;
}
