#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define PORT 33333

struct message
{
    int action;
    char fromname[20];
    char toname[20];
    char msg[1024];
};

void * recv_message(void *arg)
{
    time_t t;
    char buf[1024];
    time(&t);
    ctime_r(&t,buf);

    int ret;
    int cfd = *((int *)arg);

    struct message *msg = (struct message *)malloc(sizeof(struct message));
   
    while(1)
    {
         memset(msg,0,sizeof(struct message));

	 if((ret = recv(cfd,msg,sizeof(struct message),0)) < 0)
	 {
	     perror("recv error!");
	     exit(1);
	 }

	 if(ret == 0)
	 {
	     printf("%d is close!\n",cfd);
	     pthread_exit(NULL);
	 }
         
	 switch(msg->action)
	 {
	     case 1:
	     {
	          printf("reg succcess!\n");
		  break;
	     }
	     case 2:
	     {
	          printf("time:%srecv:%s\n",buf,msg->msg);
		  break;
	     }
	     case 3:
	     {
	          printf("time:%sall recv:%s\n",buf,msg->msg);
		  break;
	     }
	 }
	 usleep(3);
    }

     pthread_exit(NULL);
}

int main()
{
/*    time_t t;
    char buf[1024];
    time(&t);
    ctime_r(&t,buf); 
*/
    int sockfd;

    char buffer[1024];

    pthread_t id;

    struct sockaddr_in s_addr;

    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("socket error!");
	exit(1);
    }

    printf("client socket success!\n");

    bzero(&s_addr,sizeof(struct sockaddr_in));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(PORT);
    s_addr.sin_addr.s_addr = inet_addr("192.168.120.130");

    if(connect(sockfd,(struct sockaddr *)(&s_addr),sizeof(struct sockaddr_in)) < 0)
    {
        perror("connect error!");
	exit(1);
    }

    printf("connect success!\n");

    if(pthread_create(&id,NULL,recv_message,(void *)(&sockfd)) != 0)
    {
        perror("pthread create error!");
	exit(1);
    }
    
    printf("cmd:  reg  send  all!\n....");

    char cmd[20];
    char name[20];
    char toname[20];

    char message[1024];

    struct message *msg = (struct message *)malloc(sizeof(struct message));

    while(1)
    {
         printf("Please input cmd:\n");
	 scanf("%s",cmd);

	 if(strcmp(cmd,"reg") == 0)
	 {
              printf("Please input reg name:\n");
	      scanf("%s",name);

	      msg->action = 1;
	      strcpy(msg->fromname,name);
             
              if(send(sockfd,msg,sizeof(struct message),0) < 0)
	      {
	          perror("send error reg!\n");
		  exit(1);
	      }

	 }
	 if(strcmp(cmd,"send") == 0)
	 {
             printf("Please input send to name:\n");
	     scanf("%s",toname);

	     printf("Please input send message:\n");
	     scanf("%s",message);

	     msg->action = 2;
	     strcpy(msg->toname,toname);
	     strcpy(msg->msg,message);

	     send(sockfd,msg,sizeof(struct message),0);
	 }
	 if(strcmp(cmd,"all") == 0)
	 {
              printf("Please input all message:\n");
	      scanf("%s",message);

	      msg->action = 3;
	      strcpy(msg->msg,message);
	      
	      send(sockfd,msg,sizeof(struct message),0);
	 }
	 
    }
    
    shutdown(sockfd,SHUT_RDWR);

    return 0;
}
