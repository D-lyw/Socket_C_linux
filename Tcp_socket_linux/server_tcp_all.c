/*
 * @Author: D-lyw 
 * @Date: 2018-11-17 09:48:16 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-11-17 10:28:06
 * @Descripe 利用子进程和多线程处理多个客户端的连接响应请求
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

const unsigned short LOCALPORT = 3003;
const char *LOCALIP = "127.0.0.1";
#define MAXMSGSIZE 1024*5

char sendbuf[MAXMSGSIZE];
char recvbuf[MAXMSGSIZE];

// 线程描述符
pthread_t Psend;

// 发送消息线程
void *sendMsg(int fd){
    while(1){
        printf("#> ");
        fgets(sendbuf, MAXMSGSIZE, stdin);
        if(send(fd, sendbuf, MAXMSGSIZE, 0) == -1){
            fprintf(stderr, "%s\n#> ", strerror(errno));
        }
        bzero(sendbuf, MAXMSGSIZE);
    }
    return NULL;
}

int main(int argc, char const *argv[])
{
    int serverfd, recvfd;
    socklen_t sockleng;
    struct sockaddr_in serveraddr, clientaddr;
    pid_t childid;
    int perrno;

    if( (serverfd = socket(AF_INET, SOCK_STREAM, 0) ) == -1){
        fprintf(stderr, "创建服务器套接字错误, %s\n", strerror(errno));
        exit(0);
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(LOCALPORT);
    serveraddr.sin_addr.s_addr = inet_addr(LOCALIP);

    if(bind(serverfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr)) == -1){
        fprintf(stderr, "绑定套接字错误, %s\n", strerror(errno));
        exit(0);
    }

    if(listen(serverfd, 10) == -1){
        fprintf(stderr, "监听套接字错误, %s\n", strerror(errno));
    }
    printf("Listening %d port, wating connection.....\n", LOCALPORT);
    
    while(1){
        if((recvfd = accept(serverfd, (struct sockaddr *)&clientaddr, &sockleng)) == 0){
            fprintf(stderr, "连接错误, %s\n", strerror(errno));
            continue;
        }
        childid = fork();
        if(childid == 0){
            close(serverfd);
            printf("子进程<%d>: 正在处理套接字 %d \n",getpid(), recvfd);
            if((perrno = pthread_create(&Psend, NULL, sendMsg, recvfd)) != 0){
                fprintf(stderr, "创建子线程失败, %s\n", strerror(perrno));
                exit(perrno);
            }
            while(1){
                if(recv(recvfd, recvbuf, MAXMSGSIZE, 0) == -1){
                    fprintf(stderr, "Receive msg err: %s\n", strerror(errno));
                }
                if(strncmp(recvbuf, "end", 3) == 0){
                    fprintf(stdout, "***关闭%s:%d套接字的连接****\n", inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
                    close(recvbuf);
                    exit(0);
                }
                fprintf(stdout, "#(%s:%d)> %s\n", inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port), recvbuf);
            }
            close(recvfd);
            exit(0);
        }else if(childid > 0){
            printf("父进程:%d\n", childid);
        }else{
            fprintf(stderr, "fork 子进程出错，%s\n", strerror(errno));
        }
    }

    close(serverfd);
    return 0;
}
