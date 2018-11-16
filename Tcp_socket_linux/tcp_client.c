/*
 * @Author: D-lyw 
 * @Date: 2018-10-26 14:06:32 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-11-16 12:34:08
 * @name tcp_client.c
 * @descripe    实现最基本的创建套接字, 填充客户端信息,connet连接服务端, 可连续向服务端发送消息
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <string.h>
extern int errno;

#define SERVERPORT 8800

int main(int argc, char const *argv[])
{
    // 定义变量存储本地套接字描述符
    int clifd;
    // 设置本地ip地址
    const char serverIp[] = "127.0.0.1";
    // 定义套接字结构存储套接字的ip,port等信息
    struct sockaddr_in cliaddr_in;
    // 定义发送,接收缓冲区大小
    char sendBuf[1024], recvBuf[1024];

    // 创建套接字
    if((clifd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "创建套接字失败,%s\n", strerror(errno));
        exit(errno);
    }

    // 填充 服务器端结构体信息
    cliaddr_in.sin_family = AF_INET;
    cliaddr_in.sin_addr.s_addr = inet_addr(serverIp);
    cliaddr_in.sin_port = htons(SERVERPORT);

    // 请求连接服务器进程
    if(connect(clifd, (struct sockaddr *)&cliaddr_in, sizeof(struct sockaddr)) == -1){
        fprintf(stderr,"请求连接服务器失败, %s\n", strerror(errno));
        exit(errno);
    }
    strcpy(sendBuf, "hi,hi, severs!\n");
    // 发送打招呼消息
    if(send(clifd, sendBuf, 1024, 0) == -1){
        fprintf(stderr, "send message error:(, %s\n", strerror(errno));
        exit(errno);
    }
    // 阻塞等待输入,发送消息
    while(1){
        fgets(sendBuf, 1024, stdin);
        if(send(clifd, sendBuf, 1024, 0) == -1){
            fprintf(stderr, "send message error:(, %s\n", strerror(errno));
        }
    }
    close(clifd);
    return 0;
}
