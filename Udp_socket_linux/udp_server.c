#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

extern int errno;

#define SERVER_PORT 8888
#define MAXMSGSIZE 1024

// 线程描述符
pthread_t Psend;

char recvbuf[MAXMSGSIZE];
char sendbuf[MAXMSGSIZE];

// 套接字描述符
int sockfd;
struct sockaddr_in addr, clientaddr;

// 发送消息线程
void *sendMsg(void *s){
    while(1){
        fgets(sendbuf, MAXMSGSIZE, stdin);
        if(sendto(sockfd, sendbuf, MAXMSGSIZE, 0, (struct sockaddr *)&clientaddr, sizeof(struct sockaddr)) == -1){
            fprintf(stderr, "%s\n", strerror(errno));
        }
    }
    return NULL;
}

int main(int argc, char const *argv[])
{
    
    socklen_t addrlen;              // 套接字地址信息数据结构的长度

    // 创建套接字
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        fprintf(stderr, "创建套接字失败 , %s\n", strerror(errno));
    }

    bzero(&addr, sizeof(struct sockaddr_in));

    // 本地的地址信息
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(SERVER_PORT);

    // 绑定套接字,监听固定端口
    if(bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
        fprintf(stderr, "绑定套接字失败, %s\n", strerror(errno));
    }

    pthread_create(&Psend, NULL, sendMsg, NULL);

    // 阻塞接收消息
    while(1){
        if(recvfrom(sockfd, recvbuf, MAXMSGSIZE, 0, (struct sockaddr *)&clientaddr, &addrlen) == -1){
                fprintf(stderr, "Recive msg error!\n");
        }
        printf("Recive from:%s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        fprintf(stdout, "#> %s", recvbuf);
    }
    
    close(sockfd);
    return 0;
}
