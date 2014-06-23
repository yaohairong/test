#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <time.h>

int setnonblocking(int fd)
{
	int opt = fcntl(fd, F_GETFL);
	if (opt < 0)
	{
		return -1;
	}
	return fcntl(fd, F_SETFL, opt | O_NONBLOCK);
}

int main()
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		perror("socket\n");
		exit(1);
	}
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		perror("set sock opt\n");
		exit(1);
	}
	if (setnonblocking(server_fd) < 0)
	{
		perror("set non blocking\n");
		exit(1);
	}
	sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(7001);

	if (bind(server_fd, (sockaddr*)&server_addr, sizeof(sockaddr)) < 0)
	{
		perror("bind\n");
		exit(1);
	}

	if (listen(server_fd, 20) < 0)
	{
		perror("listen\n");
		exit(1);
	}
	int epoll_fd = epoll_create(20);
	if (epoll_fd < 0)
	{
		perror("epoll create\n");
		exit(1);
	}

	epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = server_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0)
	{
		perror("epoll ctl\n");
		exit(1);
	}

	char buf[1024] = {0};
	int recv_bytes = 0;
	int send_bytes = 0;
	epoll_event events[20];
	for (;;)
	{
		int n = epoll_wait(epoll_fd, events, 20, -1);
		if (n < 0 && errno != EINTR)
		{
			perror("epoll wait");
			exit(1);
		}
		for (int i = 0; i < n; ++i)
		{
			epoll_event* e = &events[i];
			if (e->data.fd == server_fd)
			{
				for (;;)
				{
					sockaddr_in client_addr;
					socklen_t addr_len = sizeof(client_addr);

					int client_fd = accept(server_fd, (sockaddr*)&client_addr, &addr_len);
					if (client_fd < 0)
					{
						if (errno != EAGAIN && errno != EINTR)
						{
							perror("accept\n");
							exit(1);
						}
						else
						{
							printf("accept block");
							break;
						}
					}
					recv_bytes = 0;
					send_bytes = 0;
					printf("client connect from %s\n", inet_ntoa(client_addr.sin_addr));
					if (setnonblocking(client_fd) < 0)
					{
						perror("set non blocking\n");
						exit(1);
					}
					ev.events = EPOLLIN | EPOLLET;
					ev.data.fd = client_fd;
					if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0)
					{
						perror("epoll ctl add");
						exit(1);
					}
				}

			}
			else if (e->events & EPOLLHUP)
			{
				printf("epoll hup client closed\n");
				close(e->data.fd);
				e->data.fd = -1;
			}
			else if (e->events & EPOLLERR)
			{
				printf("epoll err\n");
				close(e->data.fd);
				e->data.fd = -1;
			}
			else if (e->events & EPOLLIN)
			{
				for (;;)
				{
					int rn = recv(e->data.fd, buf, 1024, 0);
					if (rn == 0)
					{
						printf("client closed");
						if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, e->data.fd, &ev) < 0)
						{
							perror("epoll ctrl del");
						}
						close(e->data.fd);
						break;
					}
					else if (rn < 0)
					{
						if (errno == EINTR || errno == EAGAIN)
						{
							break;
						}
						else
						{
							perror("recv");
							if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, e->data.fd, &ev) < 0)
							{
								perror("epoll ctrl del");
							}
							close(e->data.fd);
							break;
						}
					}
					else
					{
						recv_bytes += rn;
						printf("recv bytes %d: %d\n", rn, recv_bytes);
						int left = rn;
						char* ptr = buf;
						bool writable = true;
						while (left > 0)
						{
							int sn = send(e->data.fd, ptr, left, 0);
							if (sn < 0)
							{
								if (errno == EINTR || errno == EAGAIN)
								{
									break;
								}
								else
								{
									perror("send");
									if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, e->data.fd, &ev) < 0)
									{
										perror("epoll ctrl del");
									}
									close(e->data.fd);
									writable = false;
									break;
								}
							}
							else
							{
								left -= sn;
								ptr += sn;
								send_bytes += sn;
								printf("send bytes %d\n", send_bytes);
							}
						}
						if (!writable)
						{
							break;
						}
					}
				}
			}
			else
			{
				printf("other epoll event\n");
			}
		}
	}

	close(epoll_fd);
	printf("hello world\n");
	return 0;
}
