#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 33333

struct message
{
     int action;
     char fromname[20];
     char toname[20];
     char msg[1024];
};

struct online
{
    int cfd;
    char name[20];

    struct online *next;
};

struct online *head = NULL;

void insert_user(struct online *new)
{
   if(head == NULL)
   {
        new->next = NULL;
	head = new;
   }
   else
   {
        new->next = head->next;
	head->next = new;
   }
}

int find_cfd(char *toname)
{
     if(head == NULL)
     {
        return -1;
     }

     struct online *temp = head;

     while(temp != NULL)
     {
         if(strcmp(temp->name,toname) == 0)
	 {
             return temp->cfd;
	 }

	 temp = temp->next;
     }
     return -1;

}

void * recv_message(void *arg)
{
    int ret;
    int to_cfd;
    int cfd = *((int *)arg);

    //char buffer[1024];
    struct online *new;
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
	          new = (struct online *)malloc(sizeof(struct online));
		  new->cfd = cfd;
		  strcpy(new->name,msg->fromname);

		  insert_user(new);

		  msg->action = 1;
		  send(cfd,msg,sizeof(struct message),0);
		  break;
	     }
	     case 2:
	     {
	          to_cfd = find_cfd(msg->toname);
                  
		  msg->action = 2;
		  send(to_cfd,msg,sizeof(struct message),0);
                  
		  time_t timep;
                  time(&timep);
                  char buff[100];
                  strcpy(buff,ctime(&timep));
                  buff[strlen(buff)-1] = 0;
		  
		  char record[1024];
		  sprintf(record,"%s(%s->%s):%s",buff,msg->fromname,msg->toname,msg->msg);
		  printf("one record is:%s \n",record);
		  
		  FILE *fp;		  
		  fp = fopen("a.txt","a+");
                  if(fp == NULL)
                  {
                        printf("flie open error!");
                  }else
                  {
                        fprintf(fp,"%s\n",record);
			printf("record have writen into file. \n");
                  }
                  fclose(fp);

		  break;
	     }
	     case 3:
	     {
	          struct online *temp = head;

		  while(temp != NULL)
		  {
		       to_cfd = temp->cfd;
		       msg->action = 3;
		       send(to_cfd,msg,sizeof(struct message),0);
		       temp = temp->next;
		  }
		  
		  break;
	     }
	 }
	 usleep(3);
    }

     pthread_exit(NULL);
}

void * send_message(void *arg)
{
    int ret;
    int cfd = *((int *)arg);
   

    while(1)
    {
	 if((ret = send(cfd,"hello world",12,0)) < 0)
	 {
	     perror("recv error!");
	     exit(1);
	 }

	 if(ret == 0)
	 {
	     printf("%d is close!\n",cfd);
	     pthread_exit(NULL);
	 }
        
	 sleep(1);
    }

     pthread_exit(NULL);
}

int main()
{
    int cfd;
    int sockfd;
    
    int c_len;

    char buffer[1024];

    pthread_t id;

    struct sockaddr_in s_addr;
    struct sockaddr_in c_addr;

    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("socket error!");
	exit(1);
    }

    printf("socket success!\n");
    
    int opt = 1;

    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    bzero(&s_addr,sizeof(struct sockaddr_in));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(PORT);
    s_addr.sin_addr.s_addr = inet_addr("192.168.120.130");

    if(bind(sockfd,(struct sockaddr *)(&s_addr),sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind error!");
	exit(1);
    }

    printf("bind success!\n");

    if(listen(sockfd,3) < 0)
    {
        perror("listen error!");
	exit(1);
    }

    printf("listen success!\n");

    
    while(1)
    {
         memset(buffer,0,sizeof(buffer));
	 
	 bzero(&c_addr,sizeof(struct sockaddr_in));
         c_len = sizeof(struct sockaddr_in);

         printf("accepting........!\n");
    
         if((cfd = accept(sockfd,(struct sockaddr *)(&c_addr),&c_len)) < 0)
         {
             perror("accept error!");
	     exit(1);
         }

         printf("port = %d ip = %s\n",ntohs(c_addr.sin_port),inet_ntoa(c_addr.sin_addr));

	 if(pthread_create(&id, NULL, recv_message, (void *)(&cfd)) != 0)
	 {
	     perror("pthread create error!");
	     exit(1);
	 }
         /*	 
	 if(pthread_create(&id, NULL, send_message, (void *)(&cfd)) != 0)
	 {
	     perror("pthread create error!");
	     exit(1);
	 }*/

	 usleep(3);

   }
   
    
    return 0;
}
