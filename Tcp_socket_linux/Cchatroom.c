/*
 * @Author: D-lyw 
 * @Date: 2018-11-22 21:47:58 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-11-22 23:19:58
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

struct msgHdr{
    int fd;                     // 套接字描述符
    ushort tip;                 // 0 进入聊天室,    1 离开聊天室    2 发送消息
    ushort onLineNum;           // 在线人数

};

int sockfd;
struct msgHdr *mySendMsgHdr, *myRecvMsgHdr;

void *sendMsg(void *msg){
    while(1){
        mySendMsgHdr = (struct msgHdr *)sendbuf;
        mySendMsgHdr->fd = sockfd;
        
        fgets(sendbuf + sizeof(struct msgHdr), MAXSIZE - sizeof(struct msgHdr), stdin);
        
        if(strncmp(sendbuf + sizeof(struct msgHdr), "end", 3) == 0){    // 用户离开聊天室
            mySendMsgHdr->tip = 1;      
            if(send(sockfd, sendbuf, MAXSIZE, 0) == -1){
                fprintf(stderr, "%s\n", strerror(errno));
            }
            close(sockfd);
            exit(0);
        }else{
            mySendMsgHdr->tip = 2;              // 用户发送数据
        }
        
        if(send(sockfd, sendbuf, MAXSIZE, 0) == -1){
            fprintf(stderr, "%s\n", strerror(errno));
        }
        bzero(sendbuf, MAXSIZE);
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
    
    // 接收其他用户消息
    while(1){
        // 清空缓存区
        bzero(recvbuf, MAXSIZE);
        if(recv(sockfd, recvbuf, MAXSIZE, 0) == -1){
            fprintf(stderr, "%s\n", strerror(errno));
        }
        
        myRecvMsgHdr = (struct msgHdr *)recvbuf;
        if(myRecvMsgHdr->tip == 0){
            fprintf(stdout, "        **用户 %d 加入聊天室 当前用户: %d 人**        \n", myRecvMsgHdr->fd, myRecvMsgHdr->onLineNum);
        }else if(myRecvMsgHdr->tip == 1){
            printf("       **用户 %d 离开聊天室 当前用户: %d 人**         \n", myRecvMsgHdr->fd, myRecvMsgHdr->onLineNum);
        }else if(myRecvMsgHdr->tip == 2){
            fprintf(stdout, "#%d> %s\n", myRecvMsgHdr->fd, recvbuf+sizeof(struct msgHdr));
        }
        
    }
    return 0;
}
