#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <error.h>

#define IPVERSION 4
#define ICMP_DATA_LEN 56


uint16_t packping(int sendsqe);
ushort checksum(unsigned char *buf, int len);