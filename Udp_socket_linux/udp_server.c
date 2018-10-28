#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>

#define MAXSIZE 1024

int main(int argc, char const *argv[])
{
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int sockfd;
    char buf[MAXSIZE];
    char str[INET_ADDRSTRLEN];
    int i, n;

    // 创建套接字
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        fprintf(stderr, "创建套接字失败, %s\n", strerror(errno));
    }





    return 0;
}
