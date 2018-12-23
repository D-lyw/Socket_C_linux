/*
 * @Author: D-lyw 
 * @Date: 2018-12-03 23:59:35 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-12-04 01:42:48
 * @Description  实现较完整的FTP程序功能(Linux & Socket) 客户端代码
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "./client.h"

void *recvMsg(void *sockfd){

    while(1){
        if(recv(*(int *)sockfd, recvBuf, MAXSIZE, 0) == -1){
            fprintf(stderr, "Recv msg error, %s\n", strerror(errno));
        }
        printf("%s\n", recvBuf);
        memset(recvBuf, 0, MAXSIZE);
    }

    return NULL;
}

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in remoteAddr;
    pthread_t precv;
    int perrno;

    enum COMMAND  command;

    if(argc != 2){
        fprintf(stderr, "Usage: ftp <ip address> \n");
        exit(0);
    }
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "创建套接字失败, %s\n", strerror(errno));
        exit(errno);
    }

    memset(&remoteAddr, 0, sizeof(struct sockaddr));
    remoteAddr.sin_addr.s_addr = inet_addr(argv[1]);
    remoteAddr.sin_port = htons(21);
    remoteAddr.sin_family = AF_INET;

    // 初始连接服务器
    if(connect(sockfd, (struct sockaddr *)&remoteAddr, sizeof(struct sockaddr)) == -1){
        fprintf(stderr, "连接服务器失败, %s\n", strerror(errno));
        exit(errno);
    }
    
    // 接收服务器 欢迎信息
    if(recv(sockfd, recvBuf, MAXSIZE, 0) == -1){
        fprintf(stderr, "接收数据失败, %s\n", strerror(errno));
        exit(errno);
    }
    printf("%s\n", recvBuf);
    
    // 创建接收用户消息处理线程
    if((perrno = pthread_create(&precv, NULL, recvMsg, &sockfd)) != 0){
        fprintf(stderr, "创建子消息接收线程失败, %s\n", strerror(perrno));
        exit(perrno);
    }

    char msg[100];
    int tip = -1;
    while(1){
        printf("#> ");
        fgets(sendBuf, 1024, stdin);            // 输入命令
        char *token;
        token = strtok(sendBuf, " ");           // 截取空格之前的ftp指令

        for(command = USER; command <= STAT; command++){
            if(!strcmp(instaruction[command], token)){
                tip = command;
                break;
            }
        }
        switch(tip){
            case 0 : break;
            default: break;
        }

        strcat(sendBuf, "\r\n");
        printf("%s\n", token);
        printf("%s\n", sendBuf);
        if(send(sockfd, sendBuf, MAXSIZE, 0) == -1){
            fprintf(stderr, "发送控制消息失败, %s\n", strerror(perrno));
        }
        memset(msg, 0, 1024);
    }
    return 0;
}
