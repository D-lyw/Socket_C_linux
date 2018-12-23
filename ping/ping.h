/*
 * @Author: D-lyw 
 * @Date: 2018-12-01 17:00:09 
 * @Last Modified by:   D-lyw 
 * @Last Modified time: 2018-12-01 17:00:09 
 */

#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <error.h>

#define IPVERSION 4
#define ICMP_DATA_LEN 56


int packping(int sendsqe);
ushort checksum(unsigned char *buf, int len);
int decodepack(char *buf, int len);
float timesubtract(struct timeval *begin, struct timeval *end);