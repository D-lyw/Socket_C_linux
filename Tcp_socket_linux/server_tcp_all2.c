/*
 * @Author: D-lyw 
 * @Date: 2018-11-22 17:41:01 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-11-22 20:40:40
 * @此代码 实现服务器可以主动和多个客户端进行双向通信
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
#define MAXCONNECT 100

char sendbuf[MAXMSGSIZE];
char recvbuf[MAXMSGSIZE];

// 线程描述符
pthread_t Precv;
pthread_t Psend;

int clientFdarray[MAXCONNECT];

// 服务器发送消息线程
void *sendMsg(void *arg){
    while(1){
        fgets(sendbuf, MAXMSGSIZE, stdin);
        // 通过判断发送字符串的第一个字符,决定是发个某一个套接字, 还是发个所有的套接字
        if(sendbuf[0] == '#'){                  // 发给所有的客户端
            for(int j = 0; clientFdarray[j] != 0; j++){
                if(send(clientFdarray[j], sendbuf+1, MAXMSGSIZE, 0) == -1){
                    fprintf(stderr, "%s\n", strerror(errno));
                }
                printf("%d\n", clientFdarray[j]);
            }
        }else{
            if(send(sendbuf[0] - '0', sendbuf+1, MAXMSGSIZE, 0) == -1){
                fprintf(stderr, "%s\n", strerror(errno));
            }
        }
        bzero(sendbuf, MAXMSGSIZE);
    }
    return NULL;
}

//　接收客户端消息线程
void *recvMsg(void *recvfd){
    int dealfd = *((int *)recvfd);
    while(1){
        if(recv(dealfd, recvbuf, MAXMSGSIZE, 0) == -1){
            fprintf(stderr, "Receive msg err: %s\n", strerror(errno));
        }
        if(strncmp(recvbuf, "end", 3) == 0){
            fprintf(stdout, "***客户端套接字: %d 已主动关闭!\n", dealfd);
            close(dealfd);
            return NULL;
        }
        fprintf(stdout, "(客户端:%d)> %s\n", dealfd, recvbuf);
    }
    close(dealfd);
    return NULL;
}

int main(int argc, char const *argv[])
{
    int serverfd, recvfd;
    socklen_t sockleng;
    struct sockaddr_in serveraddr, clientaddr;
    pid_t childid;
    int perrno;
    int clientNum = 0;

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

    if(listen(serverfd, 100) == -1){
        fprintf(stderr, "监听套接字错误, %s\n", strerror(errno));
    }
    
    printf("\nListening at %d port, wating connection.....\n", LOCALPORT);
    
    if(perrno = pthread_create(&Psend, NULL, sendMsg, NULL)){
        fprintf(stderr, "创建消息发送线程失败, %s\n", strerror(perrno));
        exit(perrno);
    }

    while(1){
        if((recvfd = accept(serverfd, (struct sockaddr *)&clientaddr, &sockleng)) == 0){
            fprintf(stderr, "连接错误, %s\n", strerror(errno));
            continue;
        }
        
        if((perrno = pthread_create(&Precv, NULL, recvMsg, &recvfd)) != 0){
            fprintf(stderr, "创建子消息接收线程失败, %s\n", strerror(perrno));
            exit(perrno);
        }
        clientFdarray[clientNum++] = recvfd;
        printf("%d\n", clientFdarray[clientNum-1]);
    }

    close(serverfd);
    return 0;
}
