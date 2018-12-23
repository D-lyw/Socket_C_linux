char* instaruction[] = {"USER","PASS", "SYST", "INFO", "GET", "PUT", "CD", "LS", "MKDIR", "QUIT", "PORT", "PASV", "DELE", "PWD", "STAT", "LPRT"};
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
    STAT,
    LPRT
};

#define MAXSIZE 1024
#define LOCALHOST "127.0.0.1"

char sendBuf[MAXSIZE];
char recvBuf[MAXSIZE];

void execUser(int* sockfd);
void execPass(int* sockfd);
void execSYST(int* sockfd);
void execPort(int* sockfd);
void execLPRT(int* sockfd);