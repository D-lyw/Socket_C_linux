#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
extern int errno;

#define SERVER_PORT 8888
#define MAXMSGSIZE 1024

void udpRespon(int sockfd){
    struct sockaddr_in addr;
    socklen_t addrlen;
    int n;
    char recvbuf[MAXMSGSIZE];
    char sendbuf[MAXMSGSIZE];

    while(1){
        n = recvfrom(sockfd, recvbuf, MAXMSGSIZE, 0, 
            (struct sockaddr *)&addr, &addrlen);
        recvbuf[n] = '\0';
        printf("Recive from:%s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        fprintf(stdout, "#> %s", recvbuf);
        fgets(sendbuf, MAXMSGSIZE, stdin);
        if(( n = sendto(sockfd, sendbuf, MAXMSGSIZE, 0, 
            (struct sockaddr *)&addr, sizeof(struct sockaddr))) == -1){
            fprintf(stderr, "%s\n", strerror(errno));
        }
    }
}

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in addr;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        fprintf(stderr, "创建套接字失败 , %s\n", strerror(errno));
    }

    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(SERVER_PORT);

    if(bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
        fprintf(stderr, "绑定套接字失败, %s\n", strerror(errno));
    }

    udpRespon(sockfd);
    close(sockfd);
    return 0;
}
