#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <WinSock2.h>
#pragma comment (lib, "Ws2_32.lib")
#define snprintf _snprintf
#else
#define INVALID_SOCKET -1
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <signal.h>
#endif
char* file_buf;
long file_size;
long send_count;
FILE* out_file;
FILE* in_file;
long recv_bytes;
long send_bytes;
enum ConnectState
{
	CS_DISCONNECT,
	CS_CONNECTING,
	CS_CONNECTED,
};

void Stop()
{
	exit(1);
#ifdef WIN32
	WSACleanup();
#endif
}

void Start()
{
#ifdef WIN32
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	int err;
	if ((err = WSAStartup(version, &wsaData)) != 0)
	{
		Stop();
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		Stop();
	}
#else
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		perror("signal");
		Stop();
	};
#endif
}


bool SetNonBlocking(int fd)
{
#ifdef WIN32
	u_long block = 1;
	if (ioctlsocket(fd, FIONBIO, &block) == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		return false;
	}
#else
	int flag = fcntl(fd, F_GETFL);
	if (flag == -1)
	{
		perror("fcntl F_GETFL");
	}
	if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) == -1)
	{

		perror("fcntl F_SETFL");
		return false;
	}
#endif
	return true;
}

int fd = INVALID_SOCKET;
ConnectState cs = CS_DISCONNECT;
void Connect()
{
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == INVALID_SOCKET)
	{
#ifdef WIN32
		int err = WSAGetLastError();
#else
		perror("socket\n");
#endif
		Stop();
	}

	if (!SetNonBlocking(fd))
	{
		printf("set non blocking error\n");
		Stop();
	}
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
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
#ifdef WIN32
	else if (WSAGetLastError() == WSAEWOULDBLOCK)
	{
		printf("connecting\n");
		cs = CS_CONNECTING;
	}
	else
	{
		cs = CS_DISCONNECT;
		closesocket(fd);
		fd = INVALID_SOCKET;
		int err = GetLastError();
		printf("connect failed\n");
	}
#else
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
		fd = INVALID_SOCKET;
	}
#endif
}
void Loop()
{
	char buf[1024] = {0};
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
#ifdef WIN32
			int err = WSAGetLastError();
			printf("select error");
#else
			if (errno != EINTR)
			{
				perror("select error");
				Stop();
			}
#endif
		}
		else
		{
			if (cs == CS_CONNECTING)
			{
#ifdef WIN32
				if (FD_ISSET(fd, &except_fds))
				{
					printf("except disconnect\n");
					cs = CS_DISCONNECT;
					closesocket(fd);
					fd = INVALID_SOCKET;
					return;
				}
				else
				{
					cs = CS_CONNECTED;
					send_count = 0;
					recv_bytes = 0;
					send_bytes = 0;
					continue;
				}
#else
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
						fd = INVALID_SOCKET;
						return;
					}
					else
					{
						perror("connected");
						cs = CS_CONNECTED;
						continue;
					}
#endif
				}
			}
			if (cs == CS_CONNECTED)
			{
				if (FD_ISSET(fd, &read_fds))
				{
					int rn = recv(fd, buf, 1024, 0);
					if (rn == 0)
					{
						cs = CS_DISCONNECT;
#ifdef WIN32
						int err = WSAGetLastError();
						closesocket(fd);
#else
						perror("recv closed");
						close(fd);
#endif
						fd = INVALID_SOCKET;
						return;
					}
					else if (rn < 0)
					{
#ifdef WIN32
						int err = WSAGetLastError();
						if (err != WSAEWOULDBLOCK)
						{
							cs = CS_DISCONNECT;
							printf("recv disconnect\n");
							closesocket(fd);
							fd = INVALID_SOCKET;
							return;
						}
						
#else
						if (errno != EINTR && errno != EAGAIN)
						{
							cs = CS_DISCONNECT;
							perror("recv disconnect");
							close(fd);
							fd = INVALID_SOCKET;
							return;
						}
#endif
					}
					else
					{
						recv_bytes += rn;
						printf("recv bytes %d\n", recv_bytes);
						fwrite(buf, rn, 1, out_file);
						fflush(out_file);
					}
				}
				index++;
				for ( ; ;)
				{
					int count = file_size - send_count;
					if (count > 0)
					{
						int sn = send(fd, file_buf + send_count, count, 0);
						if (sn < 0)
						{
#ifdef WIN32
							int err = WSAGetLastError();
							if (err != WSAEWOULDBLOCK)
							{
								cs = CS_DISCONNECT;
								printf("send disconnect\n");
								closesocket(fd);
								fd = INVALID_SOCKET;
								return;
							}
#else
							if (errno != EINTR && errno != EAGAIN)
							{
								perror("send disconnect");
								cs = CS_DISCONNECT;
								close(fd);
								fd = INVALID_SOCKET;
								return;
							}
#endif
							printf("send block\n");
						}
						else
						{
							send_count += sn;
							send_bytes += sn;
							printf("send bytes %d\n", send_bytes);
						}
					}
				}
			}
		}
	}

	int main(int argc, char** argv)
	{
		if (argc < 2)
		{
			exit(1);
		}
		in_file = fopen(argv[1], "rb");
		if (in_file == NULL)
		{
			exit(1);
		}
		char buf[128] = {0};
		snprintf(buf, 128, "%s.out", argv[1]);
		out_file = fopen(buf, "wb");
		if (out_file == NULL)
		{
			exit(1);
		}
		
		fseek(in_file, SEEK_SET, SEEK_END);
		file_size = ftell(in_file);
		file_buf = new char[file_size];
		fseek(in_file, SEEK_SET, 0);
		fread(file_buf, file_size, 1, in_file);

		
		Start();
		while (true)
		{
			if (cs == CS_DISCONNECT)
			{
				Connect();
			}
			else
			{
				Loop();
			}
		}
#ifdef WIN32
		WSACleanup();
#endif
		return 0;
	}
