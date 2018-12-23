char* instaruction[] = {"USER","PASS", "SYST", "INFO", "GET", "PUT", "CD", "LS", "MKDIR", "QUIT"};
// 实现的FTP指令枚举集合
enum COMMAND{
    USER, 
    PASS,
    SYST,
    INFO,
    GET,
    PUT,
    CD,
    LS,
    MKDIR,
    QUIT,
    PORT,
    PASV,
    DELE,
    PWD,
    STAT
};

#define MAXSIZE 1024

char sendBuf[MAXSIZE];
char recvBuf[MAXSIZE];