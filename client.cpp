#include "utility.h"

int main(int argc,char **argv)
{
	struct sockaddr_in serveraddr;
	serveraddr.sin_family=PF_INET;
	serveraddr.sin_port=htons(SERVER_PORT);
	serveraddr.sin_addr.s_addr=inet_addr(SERVER_IP);

	int sockfd=socket(PF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
		perror("socket error");
		exit(-1);	
	}

	if(connect(sockfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))<0)
	{
		perror("connect error");
		exit(-1);
	}
	int pipe_fd[2];
	if(pipe(pipe_fd)<0)
	{
		perror("pipe error");
		exit(-1);
	}
	int epfd=epoll_create(EPOLL_SIZE);
	if(epfd<0)
	{
		perror("epfd,error");
		exit(-1);
	}
	static struct epoll_event events[2];
	addfd(epfd,sockfd,true);
	addfd(epfd,pipe_fd[0],true);

	bool isClientWork=true;
	char message[BUF_SIZE];
	int pid=fork();
	if(pid<0)
	{
		perror("fork error");
		exit(-1);	
	}
	else if(pid==0)
	{
		//子进程负责写管道，所以先关闭读端
		close(pipe_fd[0]);
		printf("Please input 'exit' to exit the chat room.\n");
		while(isClientWork)
		{
			bzero(&message,BUF_SIZE);
			fgets(message,BUF_SIZE,stdin);

			if(strncasecmp(message,EXIT,strlen(EXIT))==0)
			{
				isClientWork=false;
			}
			else//子进程进入写管道
			{
				if(write(pipe_fd[1],message,strlen(message)-1)<0)
				{
					perror("write error");
					exit(-1);
				}
			}
		}
	}
	else
	{
		close(pipe_fd[1]);
		while(isClientWork)
		{
			int epoll_events_count=epoll_wait(epfd,events,2,-1);
			for(int i=0;i<epoll_events_count;++i)
			{
				bzero(&message,BUF_SIZE);
				if(events[i].data.fd==sockfd)
				{
					int ret=recv(sockfd,message,BUF_SIZE,0);
					if(ret==0)
					{
						printf("Server closed connection: %d\n",sockfd);
						close(sockfd);
						isClientWork=false;
					}
					else
						printf("%s\n",message);
				}
				else
				{
					int ret=read(events[i].data.fd,message,BUF_SIZE);
					if(ret==0)
						isClientWork=false;
					else
						send(sockfd,message,BUF_SIZE,0);			
				}
			}
		}
	}
	if(pid)
	{
		close(pipe_fd[0]);
		close(sockfd);
	}
	else
		close(pipe_fd[1]);

	return 0;
}

