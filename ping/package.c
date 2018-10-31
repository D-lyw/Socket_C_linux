
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/filter.h>
#include <errno.h>

#define EXIT_FIILURE -1
#define EXIT_SUCCESS 0

extern int errno;

void DisplayEthernet(struct ether_header *_pEtherheader)
{
	#if 0
	struct ether_header{  
		u_int8_t  ether_dhost[ETH_ALEN];	// destination eth addr	  
		u_int8_t  ether_shost[ETH_ALEN];	// source ether addr
		u_int16_t ether_type;		        // packet type ID field
	} __attribute__ ((__packed__));
	#endif
	char *ethertypeStr = NULL;
	unsigned short ethertype = ntohs(_pEtherheader->ether_type);
	switch(ethertype){
		case 0x0200:
			ethertypeStr = "ETHERTYPE_PUP";
			break;
		case 0x0800:
			ethertypeStr = "ETHERTYPE_IP";
			break;
		case 0x0806:
			ethertypeStr = "ETHERTYPE_ARP";
			break;
		case 0x0835:
			ethertypeStr = "ETHERTYPE_REVARP";
			break;
		default:
			ethertypeStr = "Unrecognized";
			break;
	}
	fprintf(stdout, "------this is a ethernet header------\n");
	fprintf(stdout, "ether_dhost= %02X:%02X:%02X:%02X:%02X:%02X\n", _pEtherheader->ether_dhost[0], _pEtherheader->ether_dhost[1], _pEtherheader->ether_dhost[2], _pEtherheader->ether_dhost[3], _pEtherheader->ether_dhost[4], _pEtherheader->ether_dhost[5]);
	fprintf(stdout, "ether_shost= %02X:%02X:%02X:%02X:%02X:%02X\n", _pEtherheader->ether_shost[0], _pEtherheader->ether_shost[1], _pEtherheader->ether_shost[2], _pEtherheader->ether_shost[3], _pEtherheader->ether_shost[4], _pEtherheader->ether_shost[5]);
	fprintf(stdout, "ether_type	= %#04x(%s)\n", ethertype, ethertypeStr);
	fprintf(stdout, "------end---------------------------\n");
}

void DisplayIp(struct ip *_pIPheader)
{
	#if 0
	struct ip  {
	#if __BYTE_ORDER == __LITTLE_ENDIAN    
	unsigned int ip_hl:4;		/* header length */    
	unsigned int ip_v:4;		/* version */
	#endif
	#if __BYTE_ORDER == __BIG_ENDIAN    
	unsigned int ip_v:4;		/* version */    
	unsigned int ip_hl:4;		/* header length */
	#endif    
	u_int8_t ip_tos;			/* type of service */    
	u_short ip_len;				/* total length */    
	u_short ip_id;				/* identification */    
	u_short ip_off;				/* fragment offset field */
	#define	IP_RF 0x8000			/* reserved fragment flag */
	#define	IP_DF 0x4000			/* dont fragment flag */
	#define	IP_MF 0x2000			/* more fragments flag */
	#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */    
	u_int8_t ip_ttl;			/* time to live */    
	u_int8_t ip_p;				/* protocol */    
	u_short ip_sum;				/* checksum */    
	struct in_addr ip_src, ip_dst;	/* source and dest address */  
	};
	#endif
	char *pStr = NULL;
	unsigned char p = _pIPheader->ip_p;
	switch(p){
		case 0:
			pStr = "IPPROTO_IP";
			break;
		case 1:
			pStr = "IPPROTO_ICMP";
			break;
		case 2:
			pStr = "IPPROTO_IGMP";
			break;
		case 6:
			pStr = "IPPROTO_TCP";
			break;
		case 17:
			pStr = "IPPROTO_UDP";
			break;
		default:
			pStr = "Unrecognized";
			break;
	}
	fprintf(stdout, "------this is a IP header------\n");
	fprintf(stdout, "ip_hl 		= %u\n", _pIPheader->ip_hl << 2);
	fprintf(stdout, "ip_v  		= %u\n", _pIPheader->ip_v);
	fprintf(stdout, "ip_tos  	= %u\n", _pIPheader->ip_tos);
	fprintf(stdout, "ip_len  	= %u\n", ntohs(_pIPheader->ip_len));
	fprintf(stdout, "ip_id  	= %u\n", ntohs(_pIPheader->ip_id));
	fprintf(stdout, "ip_off  	= %u\n", ntohs(_pIPheader->ip_off));
	fprintf(stdout, "ip_ttl  	= %u\n", _pIPheader->ip_ttl);
	fprintf(stdout, "ip_p  		= %u(%s)\n", p, pStr);
	fprintf(stdout, "ip_sum  	= %u\n", ntohs(_pIPheader->ip_sum));
	fprintf(stdout, "ip_src  	= %s\n", inet_ntoa(_pIPheader->ip_src));
	fprintf(stdout, "ip_dst  	= %s\n", inet_ntoa(_pIPheader->ip_dst));
	fprintf(stdout, "------end---------------------------\n");
}

void DisplayArp(struct arphdr *_pArpheader)
{
	#if 0
	struct arphdr  {    
		unsigned short int ar_hrd;		// Format of hardware address.  
		unsigned short int ar_pro;		// Format of protocol address.   
		unsigned char ar_hln;			// Length of hardware address.  
		unsigned char ar_pln;			// Length of protocol address.
		unsigned short int ar_op;		// ARP opcode (command).
	};
	#endif
	static const char *hardwaretype[] = {	"ARPHRD_NETROM", 
											"ARPHRD_ETHER",
											"ARPHRD_EETHER",
											"ARPHRD_AX25",
											"ARPHRD_PRONET",
											"ARPHRD_CHAOS",
											"ARPHRD_IEEE802",
											"ARPHRD_ARCNET",
											"ARPHRD_APPLETLK"
										};
	char *ptStr = NULL;
	unsigned short ht = ntohs(_pArpheader->ar_hrd);
	unsigned short pt = ntohs(_pArpheader->ar_pro);
	switch(pt){
		case 0x0800:
			ptStr = "IP";
			break;
		case 0x0806:
			ptStr = "ARP";
			break;
		case 0x0835:
			ptStr = "REVARP";
			break;
		default:
			ptStr = "Unrecognized";
			break;
	}

	char *opStr = NULL;
	unsigned short op = ntohs(_pArpheader->ar_op);
	switch(op){
		case 1:
			opStr = "ARPOP_REQUEST";
			break;
		case 2:
			opStr = "ARPOP_REPLY";
			break;
		default:
			opStr = "Unrecognized";
			break;
	}
	fprintf(stdout, "------this is a arp header------\n");
	fprintf(stdout, "ar_hrd 	= %#02X(%s)\n", ht, hardwaretype[ht]);
	fprintf(stdout, "ar_pro  	= %#02X(%s)\n", pt, ptStr);
	fprintf(stdout, "ar_hln  	= %u\n", _pArpheader->ar_hln);
	fprintf(stdout, "ar_pln  	= %u\n", _pArpheader->ar_pln);
	fprintf(stdout, "ar_op  	= %#02X(%s)\n", op, opStr);
	fprintf(stdout, "------end---------------------------\n");
}

void DisplayRarp(struct arphdr *_pRarpheader)
{
	static const char *hardwaretype[] = {	"ARPHRD_NETROM", 
											"ARPHRD_ETHER",
											"ARPHRD_EETHER",
											"ARPHRD_AX25",
											"ARPHRD_PRONET",
											"ARPHRD_CHAOS",
											"ARPHRD_IEEE802",
											"ARPHRD_ARCNET",
											"ARPHRD_APPLETLK"
										};
	char *ptStr = NULL;
	unsigned short ht = ntohs(_pRarpheader->ar_hrd);
	unsigned short pt = ntohs(_pRarpheader->ar_pro);
	switch(pt){
		case 0x0800:
			ptStr = "IP";
			break;
		case 0x0806:
			ptStr = "ARP";
			break;
		case 0x0835:
			ptStr = "REVARP";
			break;
		default:
			ptStr = "Unrecognized";
			break;
	}

	char *opStr = NULL;
	unsigned short op = ntohs(_pRarpheader->ar_op);
	switch(op){
		case 3:
			opStr = "ARPOP_RREQUEST";
			break;
		case 4:
			opStr = "ARPOP_RREPLY";
			break;
		default:
			opStr = "Unrecognized";
			break;
	}
	fprintf(stdout, "------this is a rarp header------\n");
	fprintf(stdout, "ar_hrd 	= %#02X(%s)\n", ht, hardwaretype[ht]);
	fprintf(stdout, "ar_pro  	= %#02X(%s)\n", pt, ptStr);
	fprintf(stdout, "ar_hln  	= %u\n", _pRarpheader->ar_hln);
	fprintf(stdout, "ar_pln  	= %u\n", _pRarpheader->ar_pln);
	fprintf(stdout, "ar_op  	= %#02X(%s)\n", op, opStr);
	fprintf(stdout, "------end---------------------------\n");
}

void DisplayTcp(struct tcphdr *_pTcpheader)
{
	#if 0
	struct tcphdr  {    
		u_int16_t source;    
		u_int16_t dest;    
		u_int32_t seq;    
		u_int32_t ack_seq;
		#  if __BYTE_ORDER == __LITTLE_ENDIAN    
		u_int16_t res1:4;    
		u_int16_t doff:4;    
		u_int16_t fin:1;    
		u_int16_t syn:1;    
		u_int16_t rst:1;    
		u_int16_t psh:1;    
		u_int16_t ack:1;    
		u_int16_t urg:1;    
		u_int16_t res2:2;
		#  elif __BYTE_ORDER == __BIG_ENDIAN    
		u_int16_t doff:4;    
		u_int16_t res1:4;    
		u_int16_t res2:2;    
		u_int16_t urg:1;    
		u_int16_t ack:1;    
		u_int16_t psh:1;    
		u_int16_t rst:1;    
		u_int16_t syn:1;    
		u_int16_t fin:1;
		#  else
		#   error "Adjust your <bits/endian.h> defines"
		#  endif    
		u_int16_t window;    
		u_int16_t check;    
		u_int16_t urg_ptr;
	};
	#endif
	fprintf(stdout, "------this is a tcp header------\n");
	fprintf(stdout, "source  	= %u\n", ntohs(_pTcpheader->source));
	fprintf(stdout, "dest  		= %u\n", ntohs(_pTcpheader->dest));
	fprintf(stdout, "seq  		= %u\n", ntohl(_pTcpheader->seq));
	fprintf(stdout, "ack_seq	= %u\n", ntohl(_pTcpheader->ack_seq));
	fprintf(stdout, "res1  		= %u\n", _pTcpheader->res1);
	fprintf(stdout, "doff  		= %u\n", _pTcpheader->doff);
	fprintf(stdout, "fin  		= %u\n", _pTcpheader->fin);
	fprintf(stdout, "syn  		= %u\n", _pTcpheader->syn);
	fprintf(stdout, "rst  		= %u\n", _pTcpheader->rst);
	fprintf(stdout, "psh  		= %u\n", _pTcpheader->psh);
	fprintf(stdout, "ack  		= %u\n", _pTcpheader->ack);
	fprintf(stdout, "urg  		= %u\n", _pTcpheader->urg);
	fprintf(stdout, "res2  		= %u\n", _pTcpheader->res2);
	fprintf(stdout, "window  	= %u\n", ntohs(_pTcpheader->window));
	fprintf(stdout, "check  	= %u\n", ntohs(_pTcpheader->check));
	fprintf(stdout, "urg  		= %u\n", ntohs(_pTcpheader->urg_ptr));
	fprintf(stdout, "------end---------------------------\n");
	
}

void DisplayUdp(struct udphdr *_pUdpheader)
{
	#if 0
	struct udphdr{  
		u_int16_t source;  
		u_int16_t dest;  
		u_int16_t len;  
		u_int16_t check;
	};
	#endif
	
	fprintf(stdout, "------this is a udp header------\n");
	fprintf(stdout, "source  	= %u\n", ntohs(_pUdpheader->source));
	fprintf(stdout, "dest  		= %u\n", ntohs(_pUdpheader->dest));
	fprintf(stdout, "len  		= %u\n", ntohs(_pUdpheader->len));
	fprintf(stdout, "check		= %u\n", ntohs(_pUdpheader->check));
	fprintf(stdout, "------end---------------------------\n");
}

void DisplayIcmp(struct icmphdr *_pIcmpheader)
{
	#if 0
	struct icmphdr{  
		u_int8_t type;		/* message type */  
		u_int8_t code;		/* type sub-code */  
		u_int16_t checksum;  
		union{    
			struct{      
				u_int16_t	id;      
				u_int16_t	sequence;    
			} echo;			/* echo datagram */    
			u_int32_t	gateway;	/* gateway address */    
			struct{      
				u_int16_t	__unused;      
				u_int16_t	mtu;    
			} frag;			/* path mtu discovery */  
		}un;
	};
	#endif
	const char *typeStr = NULL;
	const char *codeStr = NULL;
	static const char *codeDestUnreachStr[] = {	"ICMP_UNREACH_NET",
												"ICMP_UNREACH_HOST",
												"ICMP_UNREACH_PROTOCOL",
												"ICMP_UNREACH_PORT",
												"ICMP_UNREACH_NEEDFRAG",
												"ICMP_UNREACH_SRCFAIL",
												"ICMP_UNREACH_NET_UNKNOWN",
												"ICMP_UNREACH_HOST_UNKNOWN",
												"ICMP_UNREACH_ISOLATED",
												"ICMP_UNREACH_NET_PROHIB",
												"ICMP_UNREACH_HOST_PROHIB",
												"ICMP_UNREACH_TOSNET",
												"ICMP_UNREACH_TOSHOST",
												"ICMP_UNREACH_FILTER_PROHIB",
												"ICMP_UNREACH_HOST_PRECEDENCE",
												"ICMP_UNREACH_PRECEDENCE_CUTOFF"
											};
	static const char *codeRedirectStr[] = 	{	"ICMP_REDIRECT_NET",
												"ICMP_REDIRECT_HOST",
												"ICMP_REDIRECT_TOSNET",
												"ICMP_REDIRECT_TOSHOST"
											};
	static const char *codeTimeExceedStr[] = {
												"ICMP_TIMXCEED_INTRANS",
												"ICMP_TIMXCEED_REASS"
											};
	static const char *codeParaProb[] = {
												"ICMP_PARAMPROB_BADIP",
												"ICMP_PARAMPROB_OPTABSENT"
								}		;
	switch(_pIcmpheader->type){
		case 0:
			typeStr = "ICMP_ECHOREPLY";
			break;
		case 3:
			typeStr = "ICMP_DEST_UNREACH";
			if(_pIcmpheader->code <= 15){
				codeStr = codeDestUnreachStr[_pIcmpheader->code];
			}else{
				codeStr = "Unrecognized";
			}
			break;
		case 4:
			typeStr = "ICMP_SOURCE_QUENCH";
			break;
		case 5:
			typeStr = "ICMP_REDIRECT";
			if(_pIcmpheader->code <= 3){
				codeStr = codeRedirectStr[_pIcmpheader->code];
			}else{
				codeStr = "Unrecognized";
			}
			break;
		case 8:
			typeStr = "ICMP_ECHO";
			break;
		case 11:
			typeStr = "ICMP_TIME_EXCEEDED";
			if(_pIcmpheader->code <= 1){
				codeStr = codeTimeExceedStr[_pIcmpheader->code];
			}else{
				codeStr = "Unrecognized";
			}
			break;
		case 12:
			typeStr = "ICMP_PARAMETERPROB";
			if(_pIcmpheader->code <= 1){
				codeStr = codeParaProb[_pIcmpheader->code];
			}else{
				codeStr = "Unrecognized";
			}
			break;
		case 13:
			typeStr = "ICMP_TIMESTAMP";
			break;
		case 14:
			typeStr = "ICMP_TIMESTAMPREPLY";
			break;
		case 15:
			typeStr = "ICMP_INFO_REQUEST";
			break;
		case 16:
			typeStr = "ICMP_INFO_REPLY";
			break;
		case 17:
			typeStr = "ICMP_ADDRESS";
			break;
		case 18:
			typeStr = "ICMP_ADDRESSREPLY";
			break;
		default:
			typeStr = "Unrecognized";
			break;		
	}
	fprintf(stdout, "------this is a icmp header------\n");
	fprintf(stdout, "type  		= %u(%s)\n", _pIcmpheader->type, typeStr);
	fprintf(stdout, "code  		= %u(%s)\n", _pIcmpheader->code, codeStr);
	fprintf(stdout, "checksum  	= %u\n", ntohs(_pIcmpheader->checksum));
	fprintf(stdout, "------end---------------------------\n");
}

/*设置网卡为混杂模式*/
int SetIfPromisc(int _sockfd, char *_pcIfname)
{
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, _pcIfname, IF_NAMESIZE);
	if(ioctl(_sockfd, SIOCGIFFLAGS, &ifr) < 0){
		fprintf(stdout, "%03d----%02d----%s\n", __LINE__, errno, strerror(errno));
		return EXIT_FIILURE;
	}

	ifr.ifr_flags |= IFF_PROMISC;
	if(ioctl(_sockfd, SIOCSIFFLAGS, &ifr) < 0){
		fprintf(stdout, "%03d----%02d----%s\n", __LINE__, errno, strerror(errno));
		return EXIT_FIILURE;
	}

	return EXIT_SUCCESS;
}

/*取消网卡的混杂模式*/
int UnsetIfPromisc(int _sockfd, char *_pcIfname)
{
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, _pcIfname, IF_NAMESIZE);
	if(ioctl(_sockfd, SIOCGIFFLAGS, &ifr) < 0){
		fprintf(stdout, "%03d----%02d----%s\n", __LINE__, errno, strerror(errno));
		return EXIT_FIILURE;
	}

	ifr.ifr_flags &= ~IFF_PROMISC;
	if(ioctl(_sockfd, SIOCSIFFLAGS, &ifr) < 0){
		fprintf(stdout, "%03d----%02d----%s\n", __LINE__, errno, strerror(errno));
		return EXIT_FIILURE;
	}

	return EXIT_SUCCESS;
}

/*设置过滤器*/
int SetFilter(int _sockfd)
{
	/*
	BPF code generated by tcpdump -dd host 150.150.150.15
	{ 0x28, 0, 0, 0x0000000c },
	{ 0x15, 0, 4, 0x00000800 },
	{ 0x20, 0, 0, 0x0000001a },
	{ 0x15, 8, 0, 0x96969610 },
	{ 0x20, 0, 0, 0x0000001e },
	{ 0x15, 6, 7, 0x96969610 },
	{ 0x15, 1, 0, 0x00000806 },
	{ 0x15, 0, 5, 0x00008035 },
	{ 0x20, 0, 0, 0x0000001c },
	{ 0x15, 2, 0, 0x96969610 },
	{ 0x20, 0, 0, 0x00000026 },
	{ 0x15, 0, 1, 0x96969610 },
	{ 0x6, 0, 0, 0x00000060 },
	{ 0x6, 0, 0, 0x00000000 },
	*/
	struct sock_filter BPF_code[]= {
		{ 0x28, 0, 0, 0x0000000c },
		{ 0x15, 0, 4, 0x00000800 },
		{ 0x20, 0, 0, 0x0000001a },
		{ 0x15, 8, 0, 0x96969610 },
		{ 0x20, 0, 0, 0x0000001e },
		{ 0x15, 6, 7, 0x96969610 },
		{ 0x15, 1, 0, 0x00000806 },
		{ 0x15, 0, 5, 0x00008035 },
		{ 0x20, 0, 0, 0x0000001c },
		{ 0x15, 2, 0, 0x96969610 },
		{ 0x20, 0, 0, 0x00000026 },
		{ 0x15, 0, 1, 0x96969610 },
		{ 0x6, 0, 0, 0x00000060  },
		{ 0x6, 0, 0, 0x00000000  }
	};

	struct sock_fprog Filter;
	Filter.len = 14;
	Filter.filter = BPF_code;
	
	if(setsockopt(_sockfd, SOL_SOCKET, SO_ATTACH_FILTER, &Filter, sizeof(Filter))<0){
		fprintf(stdout, "%03d----%02d----%s\n", __LINE__, errno, strerror(errno));
		return EXIT_FIILURE;
	}
	
	return EXIT_SUCCESS;
}

int main()
{
	int sockfd;
	char buf[2048];
	ssize_t nRet;
	struct ether_header *etherheader;
	unsigned short ethertype;
	struct ip *ipheader;
	struct arphdr *arpheader;
	struct tcphdr *tcpheader;
	struct udphdr *udpheader;
	struct icmphdr *icmpheader;
	
	//sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	//sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(-1 == sockfd){
		fprintf(stdout, "%03d----%02d----%s\n", __LINE__, errno, strerror(errno));
		exit(EXIT_FIILURE);
	}

	if(EXIT_SUCCESS != SetIfPromisc(sockfd, "wlp3s0")){
		exit(EXIT_FIILURE);
	}

	//if(EXIT_SUCCESS != SetFilter(sockfd)){
	//	exit(EXIT_FIILURE);
	//}

	while(1){
		memset(buf, 0, 2048);
		nRet = recvfrom(sockfd, buf, 2048, 0, NULL, NULL);
		if(-1 == nRet){
			fprintf(stdout, "%03d----%02d----%s\n", __LINE__, errno, strerror(errno));
			exit(EXIT_FIILURE);
		}

		if(nRet < sizeof(struct ether_header))	{continue;}

		etherheader = (struct ether_header *)buf;
		
		DisplayEthernet(etherheader);
		
		ethertype = ntohs(etherheader->ether_type);

		if(ethertype == ETHERTYPE_IP){
			if(nRet < sizeof(struct ether_header) + sizeof(struct ip))	{continue;}

			ipheader = (struct ip *)(buf + sizeof(struct ether_header));
			DisplayIp(ipheader);

			if(ipheader->ip_p == IPPROTO_TCP){
				if(nRet < sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct tcphdr))	{continue;}

				tcpheader = (struct tcphdr *)(buf + sizeof(struct ether_header) + sizeof(struct ip));
				DisplayTcp(tcpheader);	
			}else if(ipheader->ip_p == IPPROTO_UDP){
				if(nRet < sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct udphdr))	{continue;}

				udpheader = (struct udphdr *)(buf + sizeof(struct ether_header) + sizeof(struct ip));
				DisplayUdp(udpheader);
			}else if(ipheader->ip_p == IPPROTO_ICMP){
				if(nRet < sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct icmphdr))	{continue;}

				icmpheader = (struct icmphdr *)(buf + sizeof(struct ether_header) + sizeof(struct ip));
				DisplayIcmp(icmpheader);
			}
		}else if(ethertype == ETHERTYPE_ARP){
			if(nRet < sizeof(struct ether_header) + sizeof(struct arphdr))	{continue;}

			arpheader = (struct arphdr *)(buf + sizeof(struct ether_header));
			DisplayArp(arpheader);
		}else if(ethertype == ETHERTYPE_REVARP){
			if(nRet < sizeof(struct ether_header) + sizeof(struct arphdr))	{continue;}

			arpheader = (struct arphdr *)(buf + sizeof(struct ether_header));
			DisplayRarp(arpheader);
		}else{
			fprintf(stdout, "ethertype not support:%#02x\n", ethertype);
		}
		sleep(1);
	}

	exit(EXIT_SUCCESS);

}