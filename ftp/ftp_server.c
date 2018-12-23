/*
 * @Author: D-lyw 
 * @Date: 2018-12-04 00:26:24 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-12-04 00:50:32
 * @Description  实现较完整的FTP程序功能(Linux & Socket) 服务端代码
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./server.h"
extern int errno;


int main(int argc, char const *argv[])
{
    int Ssockfd, Csockfd;
    struct sockaddr_in serverAddr, clientAddr;
    enum COMMAND command;

    if((Ssockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "创建套接字失败, %s\n", strerror(errno));
        exit(errno);
    }

    memset(&serverAddr, 0, sizeof(struct sockaddr));
    serverAddr.sin_addr.s_addr = inet_addr(LOCALHOST);
    serverAddr.sin_port = htons(21);
    serverAddr.sin_family = AF_INET;

    if(bind(Ssockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1){
        fprintf(stderr, "服务器绑定套接字失败, %s\n", strerror(errno));
        exit(errno);
    }

    if(listen(Ssockfd, 10) == -1){
        fprintf(stderr, "服务器监听端口失败, %s\n", strerror(errno));
        exit(errno);
    }
    
    printf("FTP服务器已正常开启....\n");
    
    int sockAddrLength = sizeof(struct sockaddr);
    // 服务器接收客户端连接
    Csockfd = accept(Ssockfd, (struct sockaddr *)&clientAddr, &sockAddrLength);

    strcpy(sendBuf, "220 欢迎进入FTP应用程序...\r\n");
    
    // 给客户端发送欢迎消息
    if(send(Csockfd, sendBuf, MAXSIZE, 0) == -1){
        fprintf(stderr, "欢迎消息发送失败, %s\n", strerror(errno));
    }
    memset(sendBuf, 0, MAXSIZE);

    char keys[5];
    int Tip = -1;
    while(1){
        if(recv(Csockfd, recvBuf, MAXSIZE, 0) == -1){
            fprintf(stderr, "服务器接收失败, %s\n", strerror(errno));
        }
        printf("\nOrigin: %s\n", recvBuf);

        memcpy(keys, recvBuf, 4);
        keys[4] = '\0';
        printf("____________________________\n");
        printf("服务器接收到消息: %s\n", keys);

        for(command = USER; command <= LPRT; command++){
            if(!strcmp(instaruction[command], keys)){
                Tip = command;
                break;
            }
        }
        switch(Tip) {
            case USER: execUser(&Csockfd); break;
            case PASS: execPass(&Csockfd); break;
            case SYST: execSYST(&Csockfd); break;
            case PORT: execPort(&Csockfd); break;
            case LPRT: execLPRT(&Csockfd); break;
            default: break;
        }
        Tip = -1;
    }
    return 0;
}

void execLPRT(int* sockfd){
    const char* noLPRT = "";
    sprintf(sendBuf, "%s", noLPRT);
    if(send(*sockfd, sendBuf, MAXSIZE, 0) == -1){
        fprintf(stderr, "服务器消息发送失败, %s\n", strerror(errno));
    }

    bzero(recvBuf, MAXSIZE);
}


// ftp服务器采用用主动模式
void execPort(int* sockfd){
    const char* connectOk = "200 Port command successful.";
    char* msg;
    msg = strtok(recvBuf+5, "\r\n");
    
    char ipstring[25], p1[5], p2[5];
    int dealPort;
    int i = 0, ip1 = 0 , ip2 = 0;
    int countNum =0;
    while(*msg != '\0'){
        if(countNum <= 3){      // 处理ip地址
            if(*msg == ','){
                countNum++;
                if(countNum == 4){
                    ipstring[i] = '\0';
                    printf("%s\n", ipstring);
                    i++; msg++;
                    continue;
                }else{
                    ipstring[i] = '.';
                }  
            }else{
                ipstring[i] = *msg;
            }
            i++;
            msg++;
        }else if(countNum == 4){                  // 处理端口p1
            if(*msg == ','){
                p1[ip1] = '\0';
                countNum++;
                msg++;
                continue;
            }else{
                p1[ip1++] = *msg;
            }
            msg++;
        }else if(countNum == 5){                // p2
                p2[ip2++] = *msg;
                msg++;         
        }
    }

    p2[ip2] = '\0';
    dealPort = atoi(p1)*256 + atoi(p2);
    printf("p1: %d, p2: %d\n", atoi(p1), atoi(p2));
    printf("%d\n", dealPort);
    printf("%s\n", ipstring);
    
    sprintf(sendBuf, "%s", connectOk);
    if(send(*sockfd, sendBuf, MAXSIZE, 0) == -1){
        fprintf(stderr, "服务器消息发送失败, %s\n", strerror(errno));
    }

    bzero(recvBuf, MAXSIZE);
}


void execUser(int* sockfd){
    printf("User exec\n");
    const char* judgeUser = "331 Please specify the password.";
    char* msg;
    msg = strtok(recvBuf+5, "\r\n");
    // printf("服务器接收到命令: USER %s\n", msg);
    
    sprintf(sendBuf, "%s", judgeUser);
    if(send(*sockfd, sendBuf, MAXSIZE, 0) == -1){
        fprintf(stderr, "服务器消息发送失败, %s\n", strerror(errno));
    }
    bzero(sendBuf, MAXSIZE);
    bzero(recvBuf, MAXSIZE);
}

void execPass(int* sockfd){
    printf("Pass exec\n");
    const char* judgeUserPass = "230 Login successful.";

    char* msg;
    msg = strtok(recvBuf+5, "\r\n");
    // printf("服务器接收到命令: Password %s\n", msg);

    sprintf(sendBuf, "%s", judgeUserPass);
    if(send(*sockfd, sendBuf, MAXSIZE, 0) == -1){
        fprintf(stderr, "服务器消息发送失败, %s\n", strerror(errno));
    }
    bzero(sendBuf, MAXSIZE);
    bzero(recvBuf, MAXSIZE);
}

void execSYST(int* sockfd){
    printf("SYST exec\n");
    const char* currentSYST = "215 Remote system type is UNIX.\r\n";

    sprintf(sendBuf, "%s", currentSYST);
    if(send(*sockfd, sendBuf, MAXSIZE, 0) == -1){
        fprintf(stderr, "服务器消息发送失败, %s\n", strerror(errno));
    }
    bzero(sendBuf, MAXSIZE);
    bzero(recvBuf, MAXSIZE);
}