#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <signal.h>

enum ConnectState
{
	CS_DISCONNECT,
	CS_CONNECTING,
	CS_CONNECTED,
};

int SetNonBlocking(int fd)
{
	int flag = fcntl(fd, F_GETFL);
	if (flag < 0)
	{
		return flag;
	}
	return fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}
int fd = -1;
ConnectState cs = CS_DISCONNECT;
void Connect()
{
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		printf("socket error\n");
		exit(1);
	}

	if (SetNonBlocking(fd) < 0)
	{
		printf("set non blocking error\n");
		exit(1);
	}
	sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("115.29.19.94");
	addr.sin_addr.s_addr = inet_addr("192.168.1.100");
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_addr.s_addr = inet_addr("192.168.56.101");
	//addr.sin_addr.s_addr = inet_addr("192.168.1.7");
	addr.sin_port = htons(7001);
	//addr.sin_port = htons(20000);
	int ret = connect(fd, (sockaddr*)&addr, sizeof(addr));
	if (ret == 0)
	{
		printf("connect success\n");
		cs = CS_CONNECTED;
	}
	else if (errno == EINPROGRESS)
	{
		printf("connecting\n");	
		cs = CS_CONNECTING;
	}
	else if (errno != EINTR)
	{
		cs = CS_DISCONNECT;
		close(fd);
		perror("connect failed");
		fd = -1;
	}
}
void Loop()
{
	char buf[128] = {0};
	int index = 0;
	while (true)
	{
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(fd, &read_fds);


		fd_set write_fds = read_fds;

		fd_set except_fds = read_fds;

		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		int ret = select(fd + 1, &read_fds, &write_fds, &except_fds, &timeout);
		if (ret == 0)
		{
			continue;
		}
		else if (ret < 0)
		{
			if (errno != EINTR)
			{
				perror("select error");
				exit(1);
			}
		}
		else
		{
			if (cs == CS_CONNECTING)
			{
				if (FD_ISSET(fd, &write_fds))
				{
					int opt = 0;
					socklen_t opt_len = sizeof(opt);
					printf("getsockopt\n");
					if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&opt, &opt_len) < 0 || opt != 0)
					{
						cs = CS_DISCONNECT;
						perror("disconnect");
						close(fd);
						fd = -1;
						return;
					}
					else
					{
						perror("connected");
						cs = CS_CONNECTED;
						continue;
					}
				}

			}
			if (cs == CS_CONNECTED)
			{
				if (FD_ISSET(fd, &read_fds))
				{
					int n = recv(fd, buf, 128, 0);
					if (n == 0)
					{
						perror("recv closed");
						cs = CS_DISCONNECT;
						close(fd);
						fd = -1;
						return;
					}
					else if (n < 0)
					{
						if (errno != EINTR && errno != EAGAIN)
						{
							cs = CS_DISCONNECT;
							perror("disconnect");
							close(fd);
							fd = -1;
							return;
						}
					}
					else
					{
						printf(buf);
					}
				}
				snprintf(buf, 128, "%d\n", index);
				index++;
				if (send(fd, buf, 128, 0) < 0)
				{
					if (errno != EINTR && errno != EAGAIN)
					{
						perror("disconnect");
						cs = CS_DISCONNECT;
						close(fd);
						fd = -1;
						return;
					}
				}

			}
		}
	}
}

int main()
{
	signal(SIGPIPE, SIG_IGN);
	while (true)
	{
		if (cs == CS_DISCONNECT)
		{
			sleep(3);
			Connect();
		}
		else
		{
			Loop();
		}
	}
	return 0;
}
