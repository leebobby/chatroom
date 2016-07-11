#include "utility.h"

int main(int argc,char **argv)
{
	struct sockaddr_in serveraddr;
	serveraddr.sin_family=PF_INET;
	serveraddr.sin_port=htons(SERVER_PORT);
	serveraddr.sin_addr.s_addr=inet_addr(SERVER_IP);

	int listenfd=socket(PF_INET,SOCK_STREAM,0);
	if(listenfd<0)
	{
		perror("listen error");
		exit(-1);	
	}
	printf("listen socket create!\n");
	
	if(bind(listenfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))<0)
	{
		perror("bind error");
		exit(-1);
	}
	listen(listenfd,5);
	printf("Start to listen:%s\n",SERVER_IP);

	int epfd=epoll_create(EPOLL_SIZE);
	if(epfd<0)
	{
		perror("epollcreate error");
		exit(-1);
	}
	printf("epoll created,epollfd = %d\n",epfd);
	static struct epoll_event events[EPOLL_SIZE];
	addfd(epfd,listenfd,true);

	while(1)
	{
		int epoll_events_count = epoll_wait(epfd,events,EPOLL_SIZE,-1);
		if(epoll_events_count<0)
		{
			perror("epoll_wait error");
			break;
		}
		printf("epoll_events_count = %d\n",epoll_events_count);
		
		for(int i=0;i<epoll_events_count;++i)
		{
			int sockfd=events[i].data.fd;
			if(sockfd=listenfd)
			{
				struct sockaddr_in client_address;
				socklen_t client_addrLength=sizeof(struct sockaddr_in);
				int clientfd=accept(listenfd,(struct sockaddr*)&client_address,&client_addrLength);
				printf("client connect from: %s : %d(IP:PORT),clientfd = %d \n",inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port),clientfd);
				addfd(epfd,clientfd,true);
				clients_list.push_back(clientfd);
				printf("Add new clientfd = %d to epoll\n",clientfd);
				printf("Now there are %d clients in the chat room.\n",(int)clients_list.size());
				printf("welcome message \n");
				char message[BUF_SIZE];
				bzero(message,BUF_SIZE);
				sprintf(message,SERVER_WELCOME,clientfd);
				int ret =send(clientfd,message,BUF_SIZE,0);	
				if(ret<0)
				{
					perror("send error");
					exit(-1);
				}
			}
			else
			{
				int ret=sendBroadcastMessage(sockfd);
				if(ret<0)
				{
					perror("error");
					exit(-1);
				}				
			}
		}
	}
	close(listenfd);
	close(epfd);
	return 0;
}
