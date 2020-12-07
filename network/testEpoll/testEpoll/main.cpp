#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define OPEN_MAX 100

using namespace std;

void testSelect()
{
	//监听套接字创建
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("faild to socket");
		return;
	}

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8001);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	listen(sockfd, 10);

	//保存客户端连接
	int fd_all[OPEN_MAX];
	memset(fd_all, -1, sizeof(fd_all));
	fd_all[0] = sockfd;

	//监听套接字加入select监测
	fd_set fd_read;
	FD_ZERO(&fd_read);
	FD_SET(sockfd, &fd_read);

	int i = 0, maxfd = fd_all[0];

	//设置超时时间
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	fd_set fd_select;
	struct timeval timeout_select;

	//1.监测监听套接字
	//	1.1保存客户端连接
	//	1.2客户端连接加入select监测
	//2.监测客户端连接
	//	2.1读取客户端发送过来的数据
	//	2.2	数据原样返回
	while (true)
	{
		fd_select = fd_read;
		timeout_select = timeout;

		int ret = select(maxfd + 1, &fd_select, NULL, NULL, NULL);
		cout << "select ready" << endl;
		if (ret < 0)
		{
			perror("fail to select");
		}
		if (ret == 0)
			cout << "timeout" << endl;

		//1.监听套接字监测
		if (FD_ISSET(sockfd, &fd_select))
		{
			struct sockaddr_in client_addr;
			socklen_t clientAddrLen = sizeof(client_addr);

			int connfd = accept(sockfd, (struct sockaddr*)&client_addr, &clientAddrLen);

			for (i = 0; i < OPEN_MAX; i++)
			{
				if (fd_all[i] < 0)
				{//1.1保存客户端连接
					fd_all[i] = connfd;
					break;
				}
			}

			//1.2客户端连接加入select监测
			FD_SET(connfd, &fd_read);

			if (maxfd < connfd)
				maxfd = connfd;
		}

		//2.客户端连接监测
		for (i = 1; i < maxfd; i++)
		{
			if (FD_ISSET(fd_all[i], &fd_select))
			{
				char buf[128] = { 0 };

				//2.1读取客户端发过来的数据
				int len = read(fd_all[i], buf, sizeof(buf));
				if (len < 0)
				{
					if (errno == ECONNRESET)
					{
						close(fd_all[i]);
						fd_all[i] = -1;
					}
					else
						perror("fail to read");
				}
				else if (len == 0)
				{
					close(fd_all[i]);
					fd_all[i] = -1;
				}
				else
				{//2.2数据原样返回客户端
					write(fd_all[i], buf, len);
				}
			}
		}
	}
}

void testEpoll()
{
	//监听套接字创建
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8001);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	listen(sockfd, 10);

	int fd[OPEN_MAX];
	memset(fd, -1, sizeof(fd));
	fd[0] = sockfd;

	int i = 0, maxi = 0;

	//epoll_create
	int epfd = epoll_create(10);
	if (-1 == epfd)
	{
		perror("epoll_create");
		return;
	}

	struct epoll_event event;
	struct epoll_event wait_event;

	//epoll_ctl 注册监听套接字
	event.data.fd = sockfd;
	event.events = EPOLLIN;
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);
	if (-1 == ret)
	{
		perror("epoll_ctl");
	}

	//epoll_wait
	//1.检测监听套接字
	//	1.1保存客户端连接
	//	1.2客户端连接加入epoll监测
	//2.检测客户端套接字
	//	2.1recv客户端发来的数据
	//	2.2数据原样send回客户端
	while (true)
	{
		ret = epoll_wait(epfd, &wait_event, maxi + 1, -1);
		cout << "epoll_wait ready" << endl;

		//1.检测监听套接字
		if (sockfd == wait_event.data.fd && (EPOLLIN == wait_event.events & EPOLLIN))
		{
			struct sockaddr_in client_addr;
			socklen_t clientLen = sizeof(client_addr);

			int connfd = accept(sockfd, (struct sockaddr*)&client_addr, &clientLen);

			for (i = 1; i < OPEN_MAX; i++)
			{
				if (fd[i] < 0)
				{
					//1.1保存客户端连接
					fd[i] = connfd;
					event.data.fd = connfd;
					event.events = EPOLLIN;

					//1.2客户端连接加入epoll监测
					ret = epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event);
					if (-1 == ret)
					{
						perror("epoll_ctl");
					}
					break;
				}
			}

			if (i > maxi)
				maxi = i;
		}

		//2.检测客户端套接字
		for (i = 1; i <= maxi; i++)
		{
			if (fd[i] < 0)
				continue;

			if (fd[i] == wait_event.data.fd && (EPOLLIN == wait_event.events & (EPOLLIN | EPOLLERR)))
			{
				int len = 0;
				char buf[128] = {0};

				//2.1recv客户端发来的数据
				len = recv(fd[i], buf, sizeof(buf), 0);
				if (len < 0)
				{
					if (errno == ECONNRESET)
					{
						close(fd[i]);
						fd[i] = -1;
					}
					else
						perror("read error:");
				}
				else if (len == 0)
				{
					close(fd[i]);
					fd[i] = -1;
				}
				else
				{//2.2数据原样send回客户端
					send(fd[i], buf, len, 0);
				}
			}
		}

		sleep(1);
	}
}

int main()
{
	int req = -1;
	cin >> req;

	if (req == 0)
		testSelect();
	else
		testEpoll();

	return 0;
}