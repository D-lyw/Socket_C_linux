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

![tcp_server.c](https:github.com/D-lyw/Socket_C_linux/Tcp_socket_linux/tcp_server.c)

```c
/*
 * @Author: D-lyw 
 * @Date: 2018-10-25 00:48:44 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-10-26 15:04:20
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
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

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
            recvBuf[recvLen] = '\0';
            printf("%s\n", recvBuf);
        }
        
    }
    close(recvfd);
    return 0;
}
```


![tcp_client.c](https:github.com/D-lyw/Socket_C_linux/Tcp_socket_linux/tcp_client.c)
```c
/*
 * @Author: D-lyw 
 * @Date: 2018-10-26 14:06:32 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-10-26 15:10:08
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

#define LOCALPORT 8800

int main(int argc, char const *argv[])
{
    // 定义变量存储本地套接字描述符
    int clifd;
    // 设置本地ip地址
    const char localIp[] = "127.0.0.1";
    // 定义套接字结构存储套接字的ip,port等信息
    struct sockaddr_in cliaddr_in;
    // 定义发送,接收缓冲区大小
    char sendBuf[1024], recvBuf[1024];

    // 创建套接字
    if((clifd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "创建套接字失败,%s\n", strerror(errno));
        exit(errno);
    }

    // 填充 结构体信息
    cliaddr_in.sin_family = AF_INET;
    cliaddr_in.sin_addr.s_addr = inet_addr(localIp);
    cliaddr_in.sin_port = htons(LOCALPORT);

    // 请求连接服务器进程
    if(connect(clifd, (struct sockaddr *)&cliaddr_in, sizeof(struct sockaddr)) == -1){
        fprintf(stderr,"请求连接服务器失败, %s\n", strerror(errno));
        exit(errno);
    }
    strcpy(sendBuf, "hi,hi, severs!");
    // 发送打招呼消息
    if(send(clifd, sendBuf, 1024, 0) == -1){
        fprintf(stderr, "send message error:(, %s\n", strerror(errno));
        exit(errno);
    }
    // 阻塞等待输入,发送消息
    while(1){
        scanf("%s", sendBuf);
        if(send(clifd, sendBuf, 1024, 0) == -1){
            fprintf(stderr, "send message error:(, %s\n", strerror(errno));
        }
    }
    close(clifd);
    return 0;
}
```
