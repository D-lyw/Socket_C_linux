# 学习Linux信号量及P, V操作 实例代码
<br/>

## 一. 利用信号量实现进程互斥实例

> 设有父子两个进程共享一个临界资源, 每个进程循环进入该临界资源3次,父进程每次进入临界区后显示prnt in, 出临界区则显示prnt out; 子进程每次进入临界区后显示chld in, 出临界区则显示chld out. 观察运行结果, 要求进程互斥, 应该是一个进程进来后另一个才能进入

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;设置互斥信号量mutex, 其内部标识为mutexid, 初值为1, 程序中使用睡眠延迟1秒来模拟进入临界区前和进入后所执行的程序


*程序实例代码:*

```c

/*
 * @Author: D-lyw 
 * @Date: 2018-11-07 14:37:45 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-11-10 20:59:36
 * @Descripe 利用信号量实现进程互斥的实例
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>

union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

//定义信号量标识
int mutexid;        

int main(int argc, char const *argv[])
{
    int chld, i, j;

    // 定义数据结构
    struct sembuf P, V;
    union semun arg;

    // 创建只含g一二互斥信号量u元素的信号量集
    mutexid = semget(IPC_PRIVATE, 1, 0666|IPC_CREAT);
    // 为g信号量赋初值
    arg.val = 1;
    if(semctl(mutexid, 0, SETVAL, arg) == -1)
        perror("semctl setval error\n");
    
    // 定义P, V 操作
    P.sem_num = 0;
    P.sem_op = -1;
    P.sem_flg = SEM_UNDO;
    V.sem_num = 0;
    V.sem_op = 1;
    V.sem_flg = SEM_UNDO;

    while((chld = fork()) == -1);       // 创建子进程
    if(chld > 0){                       // 父进程返回
        i = 1;
        while(i <= 3){
            sleep(1);
            semop(mutexid, &P, 1);      // 进入临界区前执行P(mutex)
            printf("prnt in \n");
            sleep(1);
            printf("prnt out\n");
            semop(mutexid, &V, 1);      // 出临界区执行V(mutex)
            i++;
        }
        wait(0);                        // 等待子进程g终止
        semctl(mutexid, IPC_RMID, 0);   // 撤销信号量
        exit(0);
    }else{                              // 子进程返回
        j = 1;
        while(j <= 3){
            sleep(1);
            semop(mutexid, &P, 1);      // 进入临界区前执行P(mutex)
            printf("chld in\n");
            sleep(1);
            printf("chld out\n");
            semop(mutexid, &V, 1);      // 出临界区执行V(mutex)
            j++;
        }
        exit(0);
    }
    
    return 0;
}

```

*程序运行结果:*

```
chld in
chld out
Print in 
Prnt out
chld in
chld out
Print in 
Prnt out
chld in
chld out
Print in 
Prnt out
```

**分析:**
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;可见, 父子进程进入临街区是有序的, 没有发生穿插进入的情况, 即每次只能有一个进程进入临界区, 这正是因为使用信号量作为互斥信号的缘故. 若不使用信号量进行,则父子进程对临界区的进入就会出现穿插的,非互斥进入的情况.


---

<br/>

## 二. 利用信号量实现进程同步

> 由父进程创建一个子进程, 父子进程共享一个存储区, 子进程向共享存储区中以覆盖方式写信息, 父进程从该共享存储区中读信息并显示信息. 父子进程轮流读写, 即子进程写一个信息到共享内存中, 父进程从中读取该信息输出; 然后子进程在第二个信息, 父进程再读出第二个信息输出, 当信息为end是读写进程结束


![单缓冲区同步问题](https://raw.githubusercontent.com/D-lyw/Notes/master/img/单缓冲区同步.png)

+ 同步分析
    这是一个单缓冲区同步问题, 读写缓冲区的两个进程之间只需要同步不需要互斥;
+ 同步算法
    子进程执行条件为单缓冲区为空, 设信号量为emptyid, 初值为1;
    父进程执行条件为单缓冲区有数, 设信号量为fullid, 初值为0;
    上述信号量可以为父进程定义, 申请, 初始化, 然后由父子进程共享使用, 子进程结束后由父进程撤销
+ 共享内存设计
    父子进程共享一个内存区, 可以由父进申请, 附接, 然后由父子进程共享使用, 子进程结束后共享内存由父进程撤销

*程序代码实例*

```c
/*
 * @Author: D-lyw 
 * @Date: 2018-11-10 20:50:01 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-11-10 20:54:33
 * @descripe 利用信号量实现进程同步 
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define BUFSIZ 1024

union semun{
    int val; 
    struct semid_ds *buf;
    unsigned short *array;
};

// 定义信号量内部标识
int emptyid;
int fullid;

int main(){
    int chld;

    // 定义信号量数据结构
    struct sembuf P, V;
    union semun arg;

    // 定义共享内存
    int shmid;
    char *viraddr;
    char buffer[BUFSIZ];

    // 创建信号量并初始化
    emptyid = semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
    fullid = semget(IPC_PRIVATE, 1, IPC_CREAT);
    arg.val = 1;
    if(semctl(emptyid, 0, SETVAL, arg) == -1)
        perror("semctl setval error");
    arg.val = 0;
    if(semctl(fullid, 0, SETVAL, arg) == -1)
        perror("semctl setval error");
    
    // 定义P, V 操作
    P.sem_num = 0;
    P.sem_op = -1;
    P.sem_flg = SEM_UNDO;
    V.sem_num = 0;
    V.sem_op = 1;
    V.sem_flg = SEM_UNDO;

    // agni创建并附接共享内存
    shmid =  shmget(IPC_PRIVATE, BUFSIZ, 0666|IPC_CREAT);
    viraddr = (char *)shmat(shmid, 0, 0);

    while ((chld = fork()) == -1);
    if(chld > 0){
        while(1){
            semop(fullid, &P, 1);                       // 对fullidx执行P 操作
            printf("Your message is: \n %s", viraddr);
            semop(emptyid, &V, 1);                      // 对emptyid执行V操作
            if(strncmp(viraddr, "end", 3) == 0)
                break;
        }
        wait(0);  
        shmdt(viraddr);
        shmctl(shmid, IPC_RMID, 0);
        semctl(emptyid, IPC_RMID, 0);
        semctl(fullid, IPC_RMID, 0);
        printf("Parent exit!\n");
        exit(0);
    }else{
        while(1){
            semop(emptyid, &P, 1);                      // 对temptyid执行P操作
            puts("Enter your text:");
            fgets(buffer, BUFSIZ, stdin);
            strcpy(viraddr, buffer);
            semop(fullid, &V, 1);                       // 对fullidin执行V操作
            if(strncmp(viraddr, "end", 3) == 0){
                sleep(1);                               // 睡眠1秒,等待父进程将"end"取走
                break;
            }
        }
        printf("child exit!\n");
        exit(0);
    }
    return 0;
}

```

由运行结果可以看出, 子进程写到单缓冲区的信息, 父进程都取出并输出了
**注意**：　在程序中，子进程在结束前的睡眠１秒是必要的，否则运行会出现不稳定规定情况，即有时正常退出，有时en不正常退出．究其原因，是父进程还没来得及取出ｅｎｄ，子进程就已经发出结束信号而影响父进程的执行．

---

<br/>

## 三. 生产者，消费者同步问题

> 设父进程创建１个子进程作为生产者，创建２个子进程作为消费者．这３个子进程使用一个共享内存，　该共享内存定义为具有５个变量的数组，　每个变量表示一个缓冲区，　缓冲区号为０－４．　生产者进程依次往缓冲区中写１０个数据１－１０，　俩个读进程依次从缓冲区０－４中轮流取出这１０个数据．使用信号量实现进程读写缓冲区的同步和互斥，如图所示：

![生产者消费者同步问题](https://raw.githubusercontent.com/D-lyw/Notes/master/img/生产者消费者同步问题.png)

#### 问题分析 :
+ 需要创建3个子进程 生产者, 消费者A, 消费者B;
+ 需要使用3个信号量empty, full , mutex, 分别表示缓冲区是否为空, 是否有数和互斥信号量, 其初值为5, 0, 1;
+ 需要2个共享内存 array, 和get , 分别表示多缓冲区数组array[0]~[4]和消费者缓冲区号的计数get, get计数由两个消费者进程共享, 由于只有一个生产者, 所以写缓冲区号计数变量set不需要使用共享内存.

#### 同步算法 :

| 生产者进程 | 消费者进程 |
| ---- | ----- |
| P(empty);| P(full) |
| P(mutex);| P(mutex) |
| 将数据送到缓冲区 | 从缓冲区读取数据 |
| V(mutex); | V(mutex) |
| V(full); | V(empty) |

由于父子进程可以共享资源, 所以3个信号量以及2个共享的内存可以在父进程中进行申请, 并完成初始化再创建子进程,在各子进程中直接使用.

#### 主程序实现算法

```c

创建共享存储区array, get;
附接共享存储区到进程空间;
共享存储区赋初值;
创建信号量empty并初始化;
创建信号量full并初始化;
创建信号量mutex并初始化;
创建生产者子进程;

如果生产者返回, 则:
for(i = 0; i < 10; i++){
    P(empty);
    P(mutex);
    写数据到共享存储区;
    缓冲区计数set++;
    V(mutex);
    V(full);
    exit(0);
}



如果父进程返回:
创建消费者A子进程;
    如果消费者A进程返回:
    for(i = 0; i < 10; i++){
        P(full);
        P(mutex);
        从共享存储区中取数据;
        缓冲区计数(*get)++;
        V(mutex);
        V(empty);
        exit(0);
    }
    如果父进程返回:
        创建消费者B子进程"
        如果消费者B进程返回:
            for(i = 0; i < 10; i++){
                P(full);
                P(mutex);
                从共享存储区中取数据;
                缓冲区计数(*get)++;
                V(mutex);
                V(empty);
                exit(0);
            }
        如果父进程返回:
            wait(0);
            wait(0);
            wait(0);
            断开2个共享存储区;
            撤销2个共享存储区;
            撤销3个信号量;
            exit(0);

```

完整实现代码:

```c
/*
 * @Author: D-lyw 
 * @Date: 2018-11-10 21:01:36 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-11-11 10:20:59
 * @Description 生产者,消费者同步问题
 */
   
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

#define MAXSHM 5                // 定义n缓冲区数组的下标变量个数

// 定义3个n信号量的内部标识
int fullid;
int emptyid;
int mutexid;

int main(int argc, char const *argv[])
{
    // 定义信号量数据结构
    struct sembuf P, V;
    union semun arg;

    // 定义2个共享内存的ID
    int arrayid;
    int getid;

    // 定义共享内存虚拟地址
    int *array;
    int *get;

    // 创建共享内存虚拟地址
    arrayid = shmget(IPC_PRIVATE, sizeof(int )*MAXSHM, IPC_CREAT|0666);
    getid = shmget(IPC_PRIVATE, sizeof(int ), IPC_CREAT|0666);

    // 初始化共享内存
    array = (int *) shmat(arrayid, 0, 0);
    get = (int *) shmat(getid, 0, 0);
    *get = 0;

    // 创建信号量
    fullid = semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
    emptyid = semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
    mutexid = semget (IPC_PRIVATE, 1, IPC_CREAT|0666);

    // 初始化信号量
    arg.val = 0;                        // 初始时缓冲区无数据
    if(semctl(fullid, 0, SETVAL, arg) == -1) 
        perror("Semctl setval error");
    arg.val = MAXSHM;                   // i初始时缓冲区有5个空闲的数组元素
    if(semctl(emptyid, 0, SETVAL, arg) == -1)
        perror("semctl setval error");
    arg.val = 1;                        // 初始时i互斥信号为1, 允许一个进程进入
    if(semctl(mutexid, 0, SETVAL, arg) == -1)
        perror("setctl setval error");
    
    // 初始化P, V操作
    P.sem_num = 0;
    P.sem_op = -1;
    P.sem_flg = SEM_UNDO;
    V.sem_num = 0;
    V.sem_op = 1;
    V.sem_flg = SEM_UNDO;

    // 生产者进程
    if(fork() == 0){
        int i = 0; 
        int set = 0;
        while(i < 10){
            semop(emptyid, &P, 1);          // 对emptyid执行P操作
            semop(mutexid, &P, 1);          // 对muteid执行P操作
            array[set%MAXSHM] = i + 1;
            printf("Producer put number %d to NO. %d \n", array[set%MAXSHM], set%MAXSHM);
            set++;                          // 写计数加1
            semop(mutexid, &V, 1);          // 对mutexid执行V操作
            semop(fullid, &V, 1);           // 对fullidn执行V操作
            i++;
        }   
        sleep(3);
        printf("Producer is over!\n");
        exit(0);
    }else{
        // 消费者A进程
        if(fork() == 0){
            while(1){
                if(*get == 10)
                    break;
                semop(fullid, &P, 1);           // 对fullid执行P操作
                semop(mutexid, &P, 1);           // 对mutexid执行P操作
                printf("The ComsumerA get number from No. %d\n", (*get)%MAXSHM);
                (*get)++;                       // 读计数加1
                semop(mutexid, &V, 1);          // 对mutexid执行V操作
                semop(emptyid, &V, 1);          // 对emptyid执行V操作
                sleep(1);
            }
            printf("ConsumerA is over\n");
            exit(0);
        }
        else{
            if(fork() == 0){
                while(1){
                    if(*get  == 10)
                        break;
                    semop(fullid, &P, 1);       // 对fullid执行P操作
                    semop(mutexid, &P, 1);      // 对mutexid执行P操作
                    printf("The comsumerB get number from No. %d \n", (*get)%MAXSHM);
                    (*get)++;
                    semop(mutexid, &V, 1);
                    semop(emptyid, &V, 1);
                    sleep(1);
                    printf("ConsumerB is over\n");
                    exit(0);
                }
            }
        }
    }

    // 父进程返回后回收3个子进程
    wait(0);
    wait(0);
    wait(0);

    // 断开并撤销2个共享内存
    shmdt(array);
    shmctl(arrayid, IPC_RMID, 0);
    shmdt(get);
    shmctl(getid, IPC_RMID, 0);

    // 撤销3个信号量集
    semctl(emptyid, IPC_RMID, 0);
    semctl(fullid, IPC_RMID, 0);
    semctl(mutexid, IPC_RMID, 0);

    exit(0);
    
    return 0;
}

```
---

<br>

## 四, 一家四口吃水果的同步问题

> 用四个线程来处理每个人对应的动作, 加上信号量来实现``同步和互斥``

```c
/*
 * @Author: D-lyw 
 * @Date: 2018-11-13 22:08:35 
 * @Last Modified by: D-lyw
 * @Last Modified time: 2018-11-14 01:15:01
 * @Descripe 
 *          桌子有一个盘子, 每次只能放入一个水果.
 *          爸爸专向盘子中放入苹果, 妈妈专向盘子中放橘子, 一个儿子专等吃盘子中的橘子, 一个女儿专吃盘子中的苹果
 *          试用PV操作实现四个人的同步
 * @Analysis
 *          爸爸,妈妈要互斥使用盘子,所以二者是互斥关系;
 *          爸爸放苹果, 女儿吃, 所以二者是同步关系;
 *          妈妈放的橘子, 儿子吃, 所以二者也是同步关系.
 *      定义三个信号量: SA, SO, mutex.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// 定义SA, SO, mutex三个信号量的内部标识
int SA;
int SO;
int mutex;

// 创建共享区标识
int plantid;
char *plant;

// 定义四个线程的线程ID
pthread_t pM;
pthread_t pF;
pthread_t pS;
pthread_t pD;

// 定义数据结构
struct sembuf P, V;
union semun arg;
    
void *mother(void *arg);
void *father(void *arg);
void *son(void *arg);
void *daughter(void *arg);

int main(int argc, char const *argv[])
{
    plantid = shmget(IPC_PRIVATE, sizeof(char )*10, IPC_CREAT|0666);
    plant = (char *) shmat(plantid, 0, 0);

    // 创建信号量
    SA = semget(IPC_PRIVATE, 1, 0666|IPC_CREAT);
    SO = semget(IPC_PRIVATE, 1, 0666|IPC_CREAT);
    mutex = semget(IPC_PRIVATE, 1, 0666|IPC_CREAT);

    // 为信号量赋初值
    arg.val = 0;
    if(semctl(SA, 0, SETVAL, arg) == -1){
        perror("semctl error");
    }
    if(semctl(SO, 0, SETVAL, arg) == -1){
        perror("semctl error");
    }
    arg.val = 1;
    if(semctl(mutex, 0, SETVAL, arg) == -1){
        perror("semctl error");
    }

    // 定义P, V操作
    P.sem_num = 0;              // 表示操作的是信号量集的第几个信号量
    P.sem_op = -1;
    P.sem_flg = SEM_UNDO;
    V.sem_num = 0;
    V.sem_op = 1;
    V.sem_flg = SEM_UNDO;

    pthread_create(&pM, NULL, mother, NULL);
    pthread_create(&pF, NULL, father, NULL);
    pthread_create(&pD, NULL, daughter, NULL);
    pthread_create(&pS, NULL, son, NULL);

    sleep(2);
    return 0;
}

void *mother(void *arg){
    int i = 3;
    while(i--){
        semop(mutex, &P, 1);
        // printf("妈妈向盘子放入一个 --橘子--\n");
        plant = "橘子";
        semop(SO, &V, 1);
    }
    return NULL;
}

void *father(void *arg){
    int i = 3; 
    while(i--){
        semop(mutex, &P, 1);
        // printf("爸爸向盘子放入一个 --苹果--\n");
        plant = "苹果";
        semop(SA, &V, 1);
    }
    return NULL;
}

void *son(void *arg){   
    int i = 3;
    while(i--){
        semop(SO, &P, 1);          // 对muteid执行P操作
        // printf("儿子从盘子中拿了一个 **橘子**\n");
        printf("儿子拿到: %s\n", plant);
        printf("--------------------------\n");
        semop(mutex, &V, 1);
    }
    return NULL;
}

void *daughter(void *arg){
    int i = 3;
    while(i--){
        semop(SA, &P, 1);
        // printf("女儿从盘子中拿了一个 **苹果**\n");
        printf("女儿拿到: %s\n", plant);
        printf("--------------------------\n");
        semop(mutex, &V, 1);
    }
    return NULL;
}

```