#include "ping.h"
#include <stdio.h>
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
struct sockaddr_in sockaddr;
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

    printf("%s \n",inet_ntoa(sockaddr.sin_addr) );
    // 创建原始套接字 SOCK_RAW 协议类型 IPPROTO_ICMP
    if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
    }

    // 设置套接字告诉内核数据中包含IP头部
    if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) == -1 ){
        fprintf(stderr, "套接字设置出错, %s\n", strerror(errno));
    }

    setuid(getpid());
    pid = getpid();


    // 发包操作
    printf("PINGing %s %d data send.\n", argv[1], ICMP_DATA_LEN);
    int i = 0;
    while(ping_time--){
        int packlen = packping(i++);
        if(sendto(sockfd, sendbuf, packlen,0, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0){
             fprintf(stderr, "send ping package %d error, %s\n", i, strerror(errno));
             continue ;
        }
        // sleep(1);
    }
    return 0;
}

// 发送ping数据包
uint16_t packping(int sendsqe){
    struct iphdr *ip_hdr;   // ip头部指针
    struct icmp *icmp_hdr;  // icmp头部指针

    // ip头部信息初始化
    ip_hdr = (struct iphdr *)sendbuf;
    ip_hdr->version = IPVERSION;
    ip_hdr->ihl = sizeof(struct iphdr)>>2;
    ip_hdr->tos = 0;
    ip_hdr->tot_len = sizeof(ip_hdr) + sizeof(icmp_hdr) + ICMP_DATA_LEN; // 报文总长度
    ip_hdr->id = 0;
    ip_hdr->frag_off = 0;
    ip_hdr->protocol = IPPROTO_ICMP;
    ip_hdr->ttl = 64;
    ip_hdr->daddr = sockaddr.sin_addr.s_addr;

    // icmp头部信息初始化
    icmp_hdr = (struct icmp *)(sendbuf + sizeof(ip_hdr));
    // icmp_hdr->icmp_type = 8;
    icmp_hdr->icmp_code = 0;
    icmp_hdr->icmp_hun.ih_idseq.icd_id = getpid();
    icmp_hdr->icmp_hun.ih_idseq.icd_seq = sendsqe;
    memset(icmp_hdr->icmp_data, 0, ICMP_DATA_LEN);
    gettimeofday((struct timeval *)icmp_hdr->icmp_data, NULL);

    int icmp_total_len = 8 + ICMP_DATA_LEN;

    icmp_hdr->icmp_cksum = 0;
    icmp_hdr->icmp_cksum = checksum((unsigned char *)(sendbuf + sizeof(ip_hdr)),icmp_total_len);

    return ip_hdr->tot_len;
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
