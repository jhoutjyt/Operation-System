/*client.c*/
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

#define PORT "3490" // Client 所要連線的 port
#define MAXDATASIZE 100 // 一次可以收到的最大位原組數（number of bytes）

// 取得 IPv4 或 IPv6 的 sockaddr：
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void *sr(void *vargp);  //啟動 傳送thread & 接收thread
void *threadsend(void *vargp);  //傳送 thread
void *threadrecv(void *vargp);  //接收 thread


int main(int argc, char *argv[])
{
  
  int sockfd, numbytes;
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];

  if (argc != 2) {
    fprintf(stderr,"usage: client hostname\n");
    exit(1);
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // 用迴圈取得全部的結果，並先連線到能成功連線的
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

  freeaddrinfo(servinfo); // 全部皆以這個 structure 完成
/*
  if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    perror("recv");
    exit(1);
  }

  buf[numbytes] = '\0';
  printf("client: received '%s'\n",buf);
*/

  int *serverfd = malloc(sizeof(int));
  *serverfd = sockfd;
  
  pthread_t tid;
  pthread_create(&tid, NULL, sr, serverfd);
  
 
  while(1){}
  close(sockfd);
  return 0;
}

void *sr(void *vargp)
{
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, threadsend, vargp);
    pthread_create(&tid2, NULL, threadrecv, vargp);

    return NULL;
}

void *threadsend(void *vargp)
{
    int connfd = *((int *)vargp);
 
    char temp[100];
    fflush(stdin);
    while(1){
        fgets(temp, 100, stdin);
        send(connfd, temp, 100, 0);
    
    }

    return NULL;
}

void *threadrecv(void *vargp)
{
    int connfd = *((int *)vargp);
    
    char temp[100];    
    while(1){
        int datalength = 0;
        datalength = recv(connfd, temp, 100 ,0);
        if(datalength > 0){
            printf("server :%s", temp);

        }
    }

    return NULL;
}
