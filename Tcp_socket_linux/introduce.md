# 基于Tcp协议的网络程序

基于tcp协议的网络程序流程图如下:

![](../img/tcp协议流程图.png)

服务器调用socket()、bind()、listen()完成初始化后，调用accept()阻塞等待，处于监听端口的状态

客户端调用socket()初始化后，调用connect()发出SYN段并阻塞等待服务器应答

服务器应答一个SYN-ACK段，客户端收到后从connect()返回，同时应答一个ACK段，服务器收到后从accept()返回。

### 数据传输的过程：

建立连接后，TCP协议提供全双工的通信服务，但是一般的客户端/服务器程序的流程是由客户端主动发起请求，服务器被动处理请求，一问一答的方式。因此，服务器从accept()返回后立刻调用read()，读socket就像读管道一样，如果没有数据到达就阻塞等待，这时客户端调用write()发送请求给服务器，服务器收到后从read()返回，对客户端的请求进行处理，在此期间客户端调用read()阻塞等待服务器的应答，服务器调用write()将处理结果发回给客户端，再次调用read()阻塞等待下一条请求，客户端收到后从read()返回，发送下一条请求，如此循环下去。
    
如果客户端没有更多的请求了，就调用close()关闭连接，就像写端关闭的管道一样，服务器的read()返回0，这样服务器就知道客户端关闭了连接，也调用close()关闭连接。注意，任何一方调用close()后，连接的两个传输方向都关闭，不能再发送数据了。如果一方调用shutdown()则连接处于半关闭状态，仍可接收对方发来的数据。

在学习socket API时要注意应用程序和TCP协议层是如何交互的： *应用程序调用某个socket函数时TCP协议层完成什么动作，比如调用connect()会发出SYN段 *应用程序如何知道TCP协议层的状态变化，比如从某个阻塞的socket函数返回就表明TCP协议收到了某些段，再比如read()返回0就表明收到了FIN段

### 简单例子

###### 客户端连接服务端后,不断从控制台读取字符串,发给服务端,服务端接收后则在控制台界面输出

[tcp_server.c](https://github.com/D-lyw/Socket_C_linux/blob/master/Tcp_socket_linux/tcp_server.c)

```c
/*
 * @Author: D-lyw 
 * @Date: 2018-10-25 00:48:44 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-12-01 23:52:03
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>

#define SERVADDR_PORT 8800

const char *LOCALIP = "127.0.0.1";

int main(int argc, char const *argv[])
{
    // 定义变量存储生成或接收的套接字描述符
    int listenfd, recvfd;
    // 定义一个数据结构用来存储套接字的协议,ip,端口等地址结构信息
    struct sockaddr_in servaddr, clientaddr;
    // 定义接收的套接字的数据结构的大小
    unsigned int cliaddr_len, recvLen;
    char recvBuf[1024];

    //创建用于帧听的套接字
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    // 给套接字数据结构赋值,指定ip地址和端口号
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVADDR_PORT);
    servaddr.sin_addr.s_addr = inet_addr(LOCALIP);

    // 绑定套接字
    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
        fprintf(stderr, "绑定套接字失败,%s\n", strerror(errno));
        exit(errno);
    }

    // 监听请求
    if(listen(listenfd, 10) == -1){
        fprintf(stderr, "绑定套接字失败,%s\n", strerror(errno));
        exit(errno);
    }

    cliaddr_len = sizeof(struct sockaddr);

    // 等待连接请求
    while (1){
        // 接受由客户机进程调用connet函数发出的连接请求
        recvfd = accept(listenfd, (struct sockaddr *)&clientaddr, &cliaddr_len);
        printf("接收到请求套接字描述符: %d\n", recvfd);

        while(1){
            // 在已建立连接的套接字上接收数据
            if((recvLen = recv(recvfd, recvBuf, 1024, 0)) == -1){
                fprintf(stderr,"接收数据错误, %s\n",strerror(errno));
            }
            printf("%s", recvBuf);
        }
    }
    close(recvfd);
    return 0;
}
```


[tcp_client.c](https://github.com/D-lyw/Socket_C_linux/blob/master/Tcp_socket_linux/tcp_client.c)
```c
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
```


### 实现一个聊天室功能

` 服务器端代码`
```c
/*
 * @Author: D-lyw 
 * @Date: 2018-11-22 20:37:05 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-11-23 00:19:42
 * @Describe Chating Room Coded by linux c .
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

const unsigned short LOCALPORT = 3003;
const char *LOCALIP = "127.0.0.1";
#define MAXMSGSIZE 1024*5
#define MAXCONNECT 100

char sendbuf[MAXMSGSIZE];
char recvbuf[MAXMSGSIZE];

struct msgHdr{
    int fd;                     // 套接字描述符
    ushort tip;                 // 0 进入聊天室,    1 离开聊天室    2 发送消息
    ushort onLineNum;           // 在线人数
};

// 线程描述符
pthread_t Precv;
pthread_t Psend;

int clientFdarray[MAXCONNECT];
struct msgHdr *sendMsgHdr, *recvMsgHdr;
int onLineNum = 0;      // 在线人数

void rmFd(int dealfd){
    for(int i = 0; i < (onLineNum+1); i++){
        if(clientFdarray[i] == dealfd){
            for(; i < onLineNum; i++){
                clientFdarray[i] = clientFdarray[i+1];
            }
            printf("当前所有用户:\n");
            for(int j = 0; j < onLineNum; j++){
                printf("     用户:%d\n", clientFdarray[j]);
            }
        }
    }
}

// 服务器发送消息线程
void sendToClient(char *buf, int dealfd){
    for(int j = 0; j < onLineNum; j++){
        if(clientFdarray[j] == dealfd){
            continue;
        }
        if(send(clientFdarray[j], buf, MAXMSGSIZE, 0) == -1){
            fprintf(stderr, "%s\n", strerror(errno));
        }
    }
    bzero(buf, MAXMSGSIZE);
}

//　接收客户端消息线程
void *recvMsg(void *recvfd){
    int dealfd = *(int *)recvfd;
    while(1){
        if(recv(dealfd, recvbuf, MAXMSGSIZE, 0) == -1){
            fprintf(stderr, "Receive msg err: %s\n", strerror(errno));
        }
        recvMsgHdr = (struct msgHdr *)recvbuf;
        recvMsgHdr->fd = dealfd;
        if(recvMsgHdr->tip == 1){
            onLineNum--;
            recvMsgHdr->onLineNum = onLineNum;
            // 将离开的套接字描述符移开在线列表数组
            rmFd(dealfd);
            printf("用户:%d 离开了聊天室\n", dealfd);
            sendToClient(recvbuf, dealfd);
            close(dealfd);
            return NULL;
        }
        // 将此用户的消息发给其他用户
        sendToClient(recvbuf, dealfd);
    }
    close(dealfd);
    return NULL;
}

int main(int argc, char const *argv[])
{
    int serverfd, recvfd;
    socklen_t sockleng;
    struct sockaddr_in serveraddr, clientaddr;
    pid_t childid;
    int perrno;

    if( (serverfd = socket(AF_INET, SOCK_STREAM, 0) ) == -1){
        fprintf(stderr, "创建服务器套接字错误, %s\n", strerror(errno));
        exit(0);
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(LOCALPORT);
    serveraddr.sin_addr.s_addr = inet_addr(LOCALIP);

    if(bind(serverfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr)) == -1){
        fprintf(stderr, "绑定套接字错误, %s\n", strerror(errno));
        exit(0);
    }

    if(listen(serverfd, 100) == -1){
        fprintf(stderr, "监听套接字错误, %s\n", strerror(errno));
    }
    
    printf("\nListening at %d port, wating connection.....\n", LOCALPORT);

    while(1){
        if((recvfd = accept(serverfd, (struct sockaddr *)&clientaddr, &sockleng)) == 0){
            fprintf(stderr, "连接错误, %s\n", strerror(errno));
            continue;
        }
        // 将该套接字描述符保存进数组
        clientFdarray[onLineNum++] = recvfd;

        printf("客户端套接字:%d 已开启\n", recvfd);

        sendMsgHdr = (struct msgHdr *)sendbuf; 
        sendMsgHdr->fd = recvfd;
        sendMsgHdr->tip = 0;
        sendMsgHdr->onLineNum = onLineNum;

        // 当有用户加入时,通知聊天室中的所有人
        for(int j = 0; j < onLineNum; j++){
            if(send(clientFdarray[j], sendbuf, MAXMSGSIZE, 0) == -1){
                fprintf(stderr, "%s\n", strerror(errno));
            }
        }
        bzero(sendbuf, MAXMSGSIZE);

        // 创建接收用户消息处理线程
        if((perrno = pthread_create(&Precv, NULL, recvMsg, &recvfd)) != 0){
            fprintf(stderr, "创建子消息接收线程失败, %s\n", strerror(perrno));
            exit(perrno);
        }
    }
    close(serverfd);
    return 0;
}

```

`客户端代码`

```c
/*
 * @Author: D-lyw 
 * @Date: 2018-11-22 21:47:58 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-11-23 00:37:01
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

extern int errno;

#define MAXSIZE 1024*5
#define SERVER_PORT 3003
const char *SERVERIP = "127.0.0.1";
// const char *SERVERIP = "120.78.156.5";

char sendbuf[MAXSIZE];
char recvbuf[MAXSIZE];

struct msgHdr{
    int fd;                     // 套接字描述符
    ushort tip;                 // 0 进入聊天室,    1 离开聊天室    2 发送消息
    ushort onLineNum;           // 在线人数

};

int sockfd;
struct msgHdr *mySendMsgHdr, *myRecvMsgHdr;

void *sendMsg(void *msg){
    while(1){
        mySendMsgHdr = (struct msgHdr *)sendbuf;
        mySendMsgHdr->fd = sockfd;
        
        fgets(sendbuf + sizeof(struct msgHdr), MAXSIZE - sizeof(struct msgHdr), stdin);
        
        if(strncmp(sendbuf + sizeof(struct msgHdr), "end", 3) == 0){    // 用户离开聊天室
            mySendMsgHdr->tip = 1;      
            if(send(sockfd, sendbuf, MAXSIZE, 0) == -1){
                fprintf(stderr, "%s\n", strerror(errno));
            }
            close(sockfd);
            exit(0);
        }else{
            mySendMsgHdr->tip = 2;              // 用户发送数据
        }
        
        if(send(sockfd, sendbuf, MAXSIZE, 0) == -1){
            fprintf(stderr, "%s\n", strerror(errno));
        }
        bzero(sendbuf, MAXSIZE);
    }
    return NULL;
}

int main(int argc, char const *argv[])
{
    ssize_t sendLen;
    struct sockaddr_in seraddr, recvaddr;
    pthread_t Psend;
    

    // 创建一个客户端的套接字
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(errno);
    }

    bzero(&seraddr, sizeof(struct sockaddr_in));
    // 服务器端地址信息
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = inet_addr(SERVERIP);
    seraddr.sin_port = htons(SERVER_PORT);

    // 请求连接服务器进程
    if(connect(sockfd, (struct sockaddr *)&seraddr, sizeof(struct sockaddr)) == -1){
        fprintf(stderr,"请求连接服务器失败, %s\n", strerror(errno));
        exit(errno);
    }
    
    printf("--------Successful connect to %s:%d--------\n", inet_ntoa(seraddr.sin_addr), ntohs(seraddr.sin_port));

    // 新建线程发送消息
    pthread_create(&Psend, NULL, sendMsg, NULL);
    
    // 接收其他用户消息
    while(1){
        // 清空缓存区
        bzero(recvbuf, MAXSIZE);
        if(recv(sockfd, recvbuf, MAXSIZE, 0) == -1){
            fprintf(stderr, "%s\n", strerror(errno));
        }
        
        myRecvMsgHdr = (struct msgHdr *)recvbuf;
        if(myRecvMsgHdr->tip == 0){
            fprintf(stdout, "        **用户 %d 加入聊天室 当前用户: %d 人**        \n", myRecvMsgHdr->fd, myRecvMsgHdr->onLineNum);
        }else if(myRecvMsgHdr->tip == 1){
            printf("       **用户 %d 离开聊天室 当前用户: %d 人**         \n", myRecvMsgHdr->fd, myRecvMsgHdr->onLineNum);
        }else if(myRecvMsgHdr->tip == 2){
            fprintf(stdout, "#%d> %s\n", myRecvMsgHdr->fd, recvbuf+sizeof(struct msgHdr));
        }
        
    }
    return 0;
}

```