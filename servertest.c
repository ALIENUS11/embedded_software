#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <winsock.h>
#include <windows.h>
#include <pthread.h>

// 定义服务器的端口号
#define PORT 33333

// 定义消息结构体
struct message
{
     int action;         // 操作类型
     char fromname[20];  // 发送者名称
     char toname[20];    // 接收者名称
     char msg[1024];     // 消息内容
};

// 定义在线用户结构体
struct online
{
    int cfd;            // 客户端文件描述符
    char name[20];      // 用户名称
    struct online *next; // 指向下一个在线用户的指针
};

struct online *head = NULL; // 在线用户链表的头指针

// 插入新用户到在线用户链表中
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

// 查找用户对应的客户端文件描述符
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

// 接收消息的线程函数
void * recv_message(void *arg)
{
    int ret;
    int to_cfd;
    int cfd = *((int *)arg);

    struct online *new;
    struct message *msg = (struct message *)malloc(sizeof(struct message));

    while(1)
    {
         memset(msg,0,sizeof(struct message)); // 清空消息结构体

         // 接收消息
         if((ret = recv(cfd,msg,sizeof(struct message),0)) < 0)
         {
             perror("recv error!");
             exit(1);
         }

         if(ret == 0)
         {
             printf("%d is close!\n",cfd); // 连接关闭
             pthread_exit(NULL); // 退出线程
         }
        
         switch(msg->action)
         {
             case 1:
             {
                  // 处理注册消息
                  new = (struct online *)malloc(sizeof(struct online));
                  new->cfd = cfd;
                  strcpy(new->name,msg->fromname);

                  insert_user(new);

                  msg->action = 1;
                  send(cfd,msg,sizeof(struct message),0); // 发送注册成功消息
                  break;
             }
             case 2:
             {
                  // 处理私信消息
                  to_cfd = find_cfd(msg->toname);
                  
                  msg->action = 2;
                  send(to_cfd,msg,sizeof(struct message),0); // 转发消息给接收者
                  
                  time_t timep;
                  time(&timep);
                  char buff[100];
                  strcpy(buff,ctime(&timep));
                  buff[strlen(buff)-1] = 0; // 去掉换行符
                  
                  char record[1024];
                  sprintf(record,"%s(%s->%s):%s",buff,msg->fromname,msg->toname,msg->msg); // 记录消息
                  printf("one record is:%s \n",record);
                  
                  FILE *fp;		  
                  fp = fopen("a.txt","a+");
                  if(fp == NULL)
                  {
                        printf("file open error!");
                  }else
                  {
                        fprintf(fp,"%s\n",record);
                        printf("record have written into file. \n");
                  }
                  fclose(fp);

                  break;
             }
             case 3:
             {
                  // 处理群发消息
                  struct online *temp = head;

                  while(temp != NULL)
                  {
                       to_cfd = temp->cfd;
                       msg->action = 3;
                       send(to_cfd,msg,sizeof(struct message),0); // 群发消息
                       temp = temp->next;
                  }
                  
                  break;
             }
         }
         usleep(3); // 休眠 3 微秒
    }

     pthread_exit(NULL); // 退出线程
}

// 发送消息的线程函数（未使用）
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

    // 创建套接字
    if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("socket error!");
        exit(1);
    }

    printf("socket success!\n");
    
    int opt = 1;

    // 设置套接字选项，允许重用地址
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    bzero(&s_addr,sizeof(struct sockaddr_in)); // 清空地址结构体
    s_addr.sin_family = AF_INET; // 设置地址族
    s_addr.sin_port = htons(PORT); // 设置端口号
    s_addr.sin_addr.s_addr = INADDR_ANY; // 设置 IP 地址为通配地址

    // 绑定套接字
    if(bind(sockfd,(struct sockaddr *)(&s_addr),sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind error!");
        exit(1);
    }

    printf("bind success!\n");

    // 监听套接字
    if(listen(sockfd,3) < 0)
    {
        perror("listen error!");
        exit(1);
    }

    printf("listen success!\n");

    while(1)
    {
         memset(buffer,0,sizeof(buffer)); // 清空缓冲区
	 
         bzero(&c_addr,sizeof(struct sockaddr_in));
         c_len = sizeof(struct sockaddr_in);

         printf("accepting........!\n");
    
         // 接受客户端连接
         if((cfd = accept(sockfd,(struct sockaddr *)(&c_addr),&c_len)) < 0)
         {
             perror("accept error!");
             exit(1);
         }

         printf("port = %d ip = %s\n",ntohs(c_addr.sin_port),inet_ntoa(c_addr.sin_addr)); // 打印客户端信息

         // 创建接收消息的线程
         if(pthread_create(&id, NULL, recv_message, (void *)(&cfd)) != 0)
         {
             perror("pthread create error!");
             exit(1);
         }

         /*
         // 创建发送消息的线程（未使用）
         if(pthread_create(&id, NULL, send_message, (void *)(&cfd)) != 0)
         {
             perror("pthread create error!");
             exit(1);
         }
         */

         usleep(3); // 休眠 3 微秒
    }

    return 0;
}
