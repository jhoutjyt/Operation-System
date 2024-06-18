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
#include <pthread.h>
#include <semaphore.h>

#define PORT "3490" //提供給使用者連線的port
#define BACKLOG 100  //20個特定的連線佇列


sem_t filesem;
int usersocks[100] = {0};
const int limit = 100;


void cleartxt()
{
        
    FILE *in;
    in = fopen("database.txt", "w");
    fclose(in);
    
}

void SendtoAll(char *sentence);
void *threadrecv(void *vargp);

int main(int argc, char *argv []){
    
    sem_init(&filesem, 0, 1);
    cleartxt(); //清空database
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
        

        for(int i = 0;i< limit;i++){
            if(usersocks[i] == 0){
                   usersocks[i] = new_fd;
                   //printf("new_fd = %d\n", new_fd);

                    
                   int *tempfd = (int *)malloc(sizeof(int));
                   *tempfd = new_fd;

                   pthread_t tid;                   
                   pthread_create(&tid, NULL, threadrecv, tempfd);
                   break;
            }    

            if(i == 100){
                send(new_fd, "The chatroom is full!", 21, 0);
                close(new_fd);
            }
            
        }


    }

  return 0;
}    


void *threadrecv(void *vargp)
{

    int connfd = *((int *)vargp);

    while(1){
        char temp [100];
        int datalength = -1;
        memset(temp,'\0', 100);

        datalength = recv(connfd, temp, 100, 0);
        
        if(datalength == 0){ //使用者離開
            int i;
            for(i =0;i<100;i++){
                if(usersocks[i] == connfd){
                    usersocks[i] = 0;
                    break;
                }    
            }
            //printf("fd = %d quit\n", connfd);
            pthread_exit((void*)i);
        }
        
        printf("%s", temp);
        SendtoAll(temp);

    }
    
    return NULL;
}


void SendtoAll(char *sentence)
{
    for(int i = 0;i<limit;i++){
        if(usersocks[i] != 0) {
            send(usersocks[i], sentence, strlen(sentence), 0);    
        }   
    }
    

    sem_wait(&filesem);
    FILE *in;
    in = fopen("database.txt", "a");
    fprintf(in, "%s", sentence);
    fclose(in);
    sem_post(&filesem);

}
