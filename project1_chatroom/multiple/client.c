/* client.c*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT "3490" // Client 所要連線的 port

char name [30]; //使用者名稱
int closeprocess = 0;

/*----取得 IPv4 或 IPv6 的 sockaddr----*/
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void *sr(void *vargp);  //啟動接收 & 傳送
void threadsend(void *vargp);   //傳送
void *threadrecv(void *vargp);  //接收

int main(int argc, char *argv[])
{
  int sockfd, numbytes;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];
    
  //檢查是否有引數，沒有的話return  
  if (argc != 2) {
    fprintf(stderr,"usage: client hostname\n");
    exit(1);
  }
  
  //設定hint
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // 用迴圈取得全部的結果，並先連線到能成功連線的addrinfo
   for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
      p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

  printf("client: connecting to %s\n", s);

  freeaddrinfo(servinfo); //釋放linklist



  //輸入名字
  printf("Enter Your name: ");
  fgets(name,40,stdin);  

  pthread_t tid;
  pthread_create(&tid, NULL, sr, &sockfd);  //啟動傳送 & 接收thread
  
 
  while(!closeprocess){}


  close(sockfd);
  return 0;
}




void *sr(void *vargp)   //傳送與開啟接收thread
{
    pthread_t tid1;
    pthread_create(&tid1, NULL, threadrecv, vargp);
    threadsend(vargp);

    return NULL;
}

void threadsend(void *vargp)    //傳送
{
    int connfd = *((int *)vargp);
    
    if(name[strlen(name) -1] =='\n') //去除換行符
        name[strlen(name)-1] = '\0';
    
    char welcome[100];  //進入聊天室
    sprintf(welcome, "Here Comes %s\n", name);
    send(connfd, welcome, 100, 0);

   
    while(1){

        setbuf(stdin, NULL);
 
        char  message[131];
        char temp[141];
        
        fgets(temp, 100, stdin);
          
        if(temp[strlen(name) -1] =='\n')
            temp[strlen(name)-1] = '\0';
        
        sprintf(message, "%s:%s", name, temp);
   
        
        if(strcmp(temp, "quit\n") == 0){
	    memset(message, '\0', 131);
            sprintf(message, "%s quit\n", name);
            send(connfd, message, strlen(message), 0);
           
            break;
	}
        
 	       
        send(connfd, message, strlen(message), 0);
    }
    close(connfd);
    closeprocess = 1;
}

void *threadrecv(void *vargp)   //接收thread
{
    int connfd = *((int *)vargp);
    
    while(1){
        char temp[100];
        memset(temp, '\0', 100);
        int datalength = 0;
        datalength = recv(connfd, temp, 100 ,0);
        if(datalength > 0)
            printf("%s", temp);
        else if(datalength == 0){
            exit(0);    
        }
            
    }

    return NULL;
}
