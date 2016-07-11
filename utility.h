#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

list<int> clients_list;//存储所有的客户的socket

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888
#define EPOLL_SIZE 5000
#define BUF_SIZE 0xFFFF
#define SERVER_WELCOME "Welcome you join to the chat room! your chat ID is: Client #%d"
#define SERVER_MESSAGE "ClientID %d say >> %s"
#define EXIT "EXIT"
#define CAUTION "There is only one in the chat room~!"

int setnonblocking(int sockfd)
{
	fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFD,0)|O_NONBLOCK);//F_GETFD 获取文件描述符 F_SETFL 设置文件状态标记
	return 0;
}

/**
  * @param epollfd: epoll handle
  * @param sockfd: socket des
  * @param enable_et: if is true ,epoll use ET;otherwise use LT
**/
void addfd(int epollfd,int sockfd,bool enable_et)
{
	struct epoll_event ev;
	ev.data.fd=sockfd;
	ev.events=EPOLLIN;
	if(enable_et)
		ev.events=EPOLLIN|EPOLLET;
	epoll_ctl(epollfd,EPOLL_CTL_ADD,sockfd,&ev);
	setnonblocking(sockfd);
	printf("sockfd added to epoll!\n\n");
}
/**
  * @param clientfd: socket des
  * @return: len
**/
int sendBroadcastMessage(int clientfd)
{
	//buf[BUF_SIZE] 接收新的聊天消息
	//message[BUF_SIZE] 保存格式消息
	char buf[BUF_SIZE],message[BUF_SIZE];
	bzero(buf,BUF_SIZE);
	bzero(message,BUF_SIZE);
	
	printf("read from client(clientID = %d)\n",clientfd);
	int len=recv(clientfd,buf,BUF_SIZE,0);
	if(len==0)//如果接收到长度为0表示客户关闭连接
	{
		close(clientfd);
		clients_list.remove(clientfd);
		printf("ClientID = %d close.\n Now there are %d client in the chat room.\n",clientfd,(int)clients_list.size());
	}
	else
	{
		if(clients_list.size()==1)
		{
			send(clientfd,CAUTION,strlen(CAUTION),0);
			return len;
		}
		sprintf(message,SERVER_MESSAGE,clientfd,buf);
		list<int>::iterator it;
		for(it=clients_list.begin();it!=clients_list.end();++it)
		{
			if(*it!=clientfd)
			{
				if(send(*it,message,BUF_SIZE,0)<0)
				{
					perror("error");
					exit(-1);
				}
			}	
		}
	}
	return len;
}
#endif
