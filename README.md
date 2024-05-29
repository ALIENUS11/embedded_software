# READ_ME

**一、 主要技术指标和要求**

1．设计一个聊天工具，可以使甲乙双方通过网络进行聊天；

2．聊天程序必须至少有一个运行在开发板上，另一个可以运行在开发板或PC均可；

3．要求能连续的实行双方聊天（即不能聊天一句之后程序结束）；

4．聊天内容实时保存在文件中，要求包含任何一条聊天信息的发送或者接收时间（年、月、日、时、分、秒）以及聊天对象；



**二.设计流程及设计思想说明**

**1.框图**

用户注册模块——用户注册信息：用户姓名

用户间私聊模块

用户信息结构体：信息发送者，信息接收者，信息

用户信息登记结构体：用户名，用户通信描述符

群聊模块——群聊消息

 

**3．子模块设计** 

**客户端注册：**

首先是服务端和客户端分别启动运行，客户端选择注册功能，向服务端发送reg指令。而此时的服务端已经开启了一个线程pthread_ create (&pid, NULL, (void .*) ServerAccept, (void *) &sockfd)来接收客户端的连接。接收到客户端的连接请求后，服务端通过函数void ServerAccept (int *serverfd) 再开辟一个子线程pthread_ create (&clientNode. pid, NULL, (voi d*) handleclient, (voi d*) &clientNode)来循环处理客户端发送过来的信息。服务端接收到封装在结构体structmessage里的指令信息reg,然后就进入int regist (struct message *recievemsg)函数。

**私聊模块：**

客户端通过who: message 的方式来发送私聊消息，服务端在void handeclient(clientinf *client)函 数中通过链表查找是否有who这个用户，如果有服务端则根据的who的套接字把message转发出去。

**群聊模块：**

群聊是以all: message 的方式发送信息的，和私聊的主要区别就是，这里要将链表的所有用户查找出来并且转发信息。
