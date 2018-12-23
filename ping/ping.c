/*
 * @Author: D-lyw 
 * @Date: 2018-11-01 17:00:20 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-12-01 23:33:49
 * @Description 在Linux环境利用socket编程,基于ICMP协议实现ping功能
 */

#include "ping.h"
#include <stdio.h>d
#include <string.h>
#include <netdb.h>            // struct icpmhdr, struct iphdr , gethostbyname, hostent
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>

char sendbuf[1024];          // 用来存放将要发送的ip数据包
struct sockaddr_in sockaddr, recvsock;
int sockaddr_len = sizeof(struct sockaddr);
struct hostent *host;
int sockfd;
int ping_time = 5;

int main(int argc, char const *argv[])
{
    if(argc != 2){
        fprintf(stderr, "Usage: ping <remote_hostname>\n", argv[0]);
        exit(1);
    }

    int on = 1;
    int pid;
    int psend = 0, precv = 0;

    memset(&sockaddr, 0, sizeof(struct sockaddr));
    if((sockaddr.sin_addr.s_addr = inet_addr(argv[1])) == INADDR_NONE){
        // 说明输入的主机名不是点分十进制,采用域名方式解析
        if((host = gethostbyname(argv[1])) == NULL){
            fprintf(stderr, "ping %s , 未知的名称!\n", argv[1]);
            exit(1);
        }
        sockaddr.sin_addr = *(struct in_addr *)(host->h_addr);
    }
    sockaddr.sin_family = AF_INET;


    // 创建原始套接字 SOCK_RAW 协议类型 IPPROTO_ICMP
    if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
    }

    setuid(getpid());
    pid = getpid();

    // 发包操作
    printf("PINGing %s %d data send.\n", argv[1], ICMP_DATA_LEN);
    int i = 1;
    int recvDataLen;
    int sendDatalen;
    char *recvbuf[1024];
    while(ping_time--){
        int packlen = packping(i++);
        if((sendDatalen = sendto(sockfd, sendbuf, packlen,0, (struct sockaddr *)&sockaddr, sizeof(sockaddr))) < 0){
             fprintf(stderr, "send ping package %d error, %s\n", i, strerror(errno));
             continue ;
        }

        if((recvDataLen = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&recvsock, &sockaddr_len)) == -1){
            fprintf(stderr, "recvmsg error, %s\n", strerror(errno));
            continue;
        }
        decodepack(recvbuf, recvDataLen);
        sleep(1);
    }


    return 0;
}

// 发送ping数据包
int packping(int sendsqe){
    struct icmp *icmp_hdr;  // icmp头部指针

    icmp_hdr = (struct icmp *)sendbuf;
    icmp_hdr->icmp_type = ICMP_ECHO;
    icmp_hdr->icmp_code = 0;
    icmp_hdr->icmp_hun.ih_idseq.icd_id = getpid();
    icmp_hdr->icmp_hun.ih_idseq.icd_seq = sendsqe;
    memset(icmp_hdr->icmp_data, 0, ICMP_DATA_LEN);
    gettimeofday((struct timeval *)icmp_hdr->icmp_data, NULL);

    int icmp_total_len = 8 + ICMP_DATA_LEN;

    icmp_hdr->icmp_cksum = 0;
    icmp_hdr->icmp_cksum = checksum((unsigned char *)(sendbuf),icmp_total_len);

    return icmp_total_len;
}

int decodepack(char *buf, int len){
    struct iphdr *ip_hdr;
    struct icmp *icmp_hdr;
    int iph_lenthg;
    float rtt; // 往返时间  
    struct timeval end; // 接收报文时的时间戳

    ip_hdr = (struct iphdr *)buf;
    // ip头部长度
    iph_lenthg = ip_hdr->ihl<<2;
    
    icmp_hdr = (struct icmp *)(buf + iph_lenthg);

    // icmp报文的长度
    len -= iph_lenthg;
    if(len < 8){
        fprintf(stderr, "Icmp package length less 8 bytes , error!\n");
        return -1;
    }

    // 确认是本机发出的icmp报文的响应
    if(icmp_hdr->icmp_type != ICMP_ECHOREPLY || icmp_hdr->icmp_hun.ih_idseq.icd_id != getpid()){
        fprintf(stderr, "Don't send to us!");
        return -1;
    }
    gettimeofday(&end, NULL);
    rtt = timesubtract((struct timeval *)&icmp_hdr->icmp_data, &end);
    printf("Received %d bytes from %s, ttl = %d, rtt = %f ms, icmpseq = %d \n", len, inet_ntoa(recvsock.sin_addr),ip_hdr->ttl, rtt, icmp_hdr->icmp_seq);

    return 0;
}

// 计算时间差
float timesubtract(struct timeval *begin, struct timeval *end){
    int n;// 先计算两个时间点相差多少微秒
    n = ( end->tv_sec - begin->tv_sec ) * 1000000
        + ( end->tv_usec - begin->tv_usec );
    // 转化为毫秒返回
    return (float) (n / 1000);
}

// 校验和生成
ushort checksum(unsigned char *buf, int len){
    unsigned int sum=0;
    unsigned short *cbuf;

    cbuf=(unsigned short *)buf;

    while(len>1){
        sum+=*cbuf++;
        len-=2;
    }

    if(len)
        sum+=*(unsigned char *)cbuf;

    sum=(sum>>16)+(sum & 0xffff);
    sum+=(sum>>16);

    return ~sum;
}
