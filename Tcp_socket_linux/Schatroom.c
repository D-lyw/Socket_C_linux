/*
 * @Author: D-lyw 
 * @Date: 2018-11-22 20:37:05 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-11-23 00:19:42
 * @Describe Chating Room Coded by linux c .
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

struct msgHdr{
    int fd;                     // 套接字描述符
    ushort tip;                 // 0 进入聊天室,    1 离开聊天室    2 发送消息
    ushort onLineNum;           // 在线人数
};

// 线程描述符
pthread_t Precv;
pthread_t Psend;

int clientFdarray[MAXCONNECT];
struct msgHdr *sendMsgHdr, *recvMsgHdr;
int onLineNum = 0;      // 在线人数

void rmFd(int dealfd){
    for(int i = 0; i < (onLineNum+1); i++){
        if(clientFdarray[i] == dealfd){
            for(; i < onLineNum; i++){
                clientFdarray[i] = clientFdarray[i+1];
            }
            printf("当前所有用户:\n");
            for(int j = 0; j < onLineNum; j++){
                printf("     用户:%d\n", clientFdarray[j]);
            }
        }
    }
}

// 服务器发送消息线程
void sendToClient(char *buf, int dealfd){
    for(int j = 0; j < onLineNum; j++){
        if(clientFdarray[j] == dealfd){
            continue;
        }
        if(send(clientFdarray[j], buf, MAXMSGSIZE, 0) == -1){
            fprintf(stderr, "%s\n", strerror(errno));
        }
    }
    bzero(buf, MAXMSGSIZE);
}

//　接收客户端消息线程
void *recvMsg(void *recvfd){
    int dealfd = *(int *)recvfd;
    while(1){
        if(recv(dealfd, recvbuf, MAXMSGSIZE, 0) == -1){
            fprintf(stderr, "Receive msg err: %s\n", strerror(errno));
        }
        recvMsgHdr = (struct msgHdr *)recvbuf;
        recvMsgHdr->fd = dealfd;
        if(recvMsgHdr->tip == 1){
            onLineNum--;
            recvMsgHdr->onLineNum = onLineNum;
            // 将离开的套接字描述符移开在线列表数组
            rmFd(dealfd);
            printf("用户:%d 离开了聊天室\n", dealfd);
            sendToClient(recvbuf, dealfd);
            close(dealfd);
            return NULL;
        }
        // 将此用户的消息发给其他用户
        sendToClient(recvbuf, dealfd);
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

    while(1){
        if((recvfd = accept(serverfd, (struct sockaddr *)&clientaddr, &sockleng)) == 0){
            fprintf(stderr, "连接错误, %s\n", strerror(errno));
            continue;
        }
        // 将该套接字描述符保存进数组
        clientFdarray[onLineNum++] = recvfd;

        printf("客户端套接字:%d 已开启\n", recvfd);

        sendMsgHdr = (struct msgHdr *)sendbuf; 
        sendMsgHdr->fd = recvfd;
        sendMsgHdr->tip = 0;
        sendMsgHdr->onLineNum = onLineNum;

        // 当有用户加入时,通知聊天室中的所有人
        for(int j = 0; j < onLineNum; j++){
            if(send(clientFdarray[j], sendbuf, MAXMSGSIZE, 0) == -1){
                fprintf(stderr, "%s\n", strerror(errno));
            }
        }
        bzero(sendbuf, MAXMSGSIZE);

        // 创建接收用户消息处理线程
        if((perrno = pthread_create(&Precv, NULL, recvMsg, &recvfd)) != 0){
            fprintf(stderr, "创建子消息接收线程失败, %s\n", strerror(perrno));
            exit(perrno);
        }
    }
    close(serverfd);
    return 0;
}
