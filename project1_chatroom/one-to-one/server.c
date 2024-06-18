#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>  //定義struct addrinfo, getaddrinfo 函式
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>

#define PORT "3490" //提供給使用者連線的port
#define BACKLOG 20  //20個特定的連線佇列

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);    
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);        
}

void cleartxt()
{
        
    FILE *in;
    in = fopen("database.txt", "w");
    fclose(in);
    
}

void *sr(void *vargp);
void *threadsend(void *vargp);
void *threadrecv(void *vargp);

int main(int argc, char *argv []){
    
    cleartxt();
    int yes = 1;

    int sockfd; //listen的檔案描述符
    int new_fd; //儲存新的連線的檔案描述符

    struct sockaddr_storage user_addr; //使用者位址資訊



    /*----設定structure----*/
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    
    //設定hint，找到符合哪些條件的structure
    memset(&hints, 0, sizeof hints); //清空hint
    hints.ai_family = AF_UNSPEC;    //用IPv4或IPv6皆可
    hints.ai_socktype = SOCK_STREAM;    //TCP stream sockets
    hints.ai_flags = AI_PASSIVE;    //將本機位置指定給hints

    //呼叫getaddrinfo尋找
    if((status = getaddrinfo(NULL, PORT, &hints, &servinfo) != 0)){
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
    

    /*----建立socket，並綁定socket與IP PORT----*/
    //綁定到一個可以使用的
    struct addrinfo *p;    
    char ipstr[INET6_ADDRSTRLEN];
    for(p = servinfo;p!=NULL;p=p->ai_next){
        
        //建立socket時發生錯誤
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server:socket");
            continue;
              
          }

        //設定socket發生錯誤（目的在防止Address already in use錯誤訊息）
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }
        
        //將socket bind到structure內的IP與PORT時 發生錯誤
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == 1){
            perror("server:bind");
            close(sockfd);
            continue;
        }
        break;
    }
    

    //如果找不到可以用的structure，結束程式
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    } 
    
    //bind好後可以free掉動態配置的servinfo指針
    freeaddrinfo(servinfo);


    /*----Listern----*/
    //如果Listen失敗，中止程式
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    
    /*
    // 收拾全部死掉的 processes
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

     if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
     }
     */

    printf("server: waitiing for connections...\n");
    
    
    //持續監聽是否有接收到連線
    socklen_t sin_size;
    char s [INET6_ADDRSTRLEN];

    while(1) { // 主要的 accept() 迴圈
        sin_size = sizeof user_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&user_addr, &sin_size);
        
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        else{
            inet_ntop(user_addr.ss_family, get_in_addr((struct sockaddr *)&user_addr), s, sizeof s);
            printf("server: got connection from %s\n", s);

        
            /*
            if (send(new_fd, "Hello, world\n", 100, 0) == -1)
                perror("send");
            */
            //close(new_fd); // parent 不需要這個
            break;
        }
    }

    //完成接收
    
    //threadpthread用的封包
    int* tempfd = (int *)malloc(sizeof(int));
    *tempfd = new_fd;

    pthread_t tid;
    pthread_create(&tid, NULL, sr, tempfd);
    
    while(1){};
  return 0;
}    

void *sr(void *vargp)
{   
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, threadsend, vargp);
    pthread_create(&tid2, NULL, threadrecv, vargp);

    return NULL;
}

void *threadrecv(void *vargp)
{
    char temp[100];
    int connfd = *((int *)vargp);
    
    while(1){
        int datalength = -1;
        datalength = recv(connfd, temp, 100, 0);
        
        if(datalength > 0){
            printf("client :%s", temp);
        
            FILE *in;
            in = fopen("database.txt", "a");
            fprintf(in, "client: %s", temp);
            fclose(in);
        }

    }
    
    return NULL;
}


void *threadsend(void *vargp)
{
    char temp[100];
    int connfd = *((int *)vargp);
    

    while(1){
        fflush(stdin);
        fgets(temp, 100, stdin);
        if (send(connfd, temp, 100, 0) == -1)
            perror("send");
        
        FILE *in;
        in = fopen("database.txt", "a");
        fprintf(in, "server: %s", temp);
        fclose(in);
    }
    
    return NULL;
}
