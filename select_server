#pragma once
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#include <list>
#include <algorithm>

using namespace std;

int main()
{
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	if (WSAStartup(version, &wsaData) == 0)
	{
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		{
			return -1;
		}
	}

	SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == INVALID_SOCKET)
	{
		return -1;
	}
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	//addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(6001);

	if (bind(fd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		return -1;
	}
	if (listen(fd, 5) == SOCKET_ERROR)
	{
		return -1;
	}
	struct Client
	{
		SOCKET fd;
		int data;
		bool connected;
	}client_list[3];
	for (auto& v : client_list)
	{
		v.connected = false;
	}
	while (true)
	{
		fd_set rset;
		fd_set wset;
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(fd, &rset);
		for (auto& v : client_list)
		{
			if (v.connected)
			{
				FD_SET(v.fd, &rset);
				//FD_SET(v.fd, &wset);
			}
		}
		int ret = select(0, &rset, &wset, NULL, NULL);
		if (ret < 0)
		{
			return -1;
		}
		if (ret == 0)
		{
			continue;
		}
		if (FD_ISSET(fd, &rset))
		{
			SOCKET client_fd = accept(fd, NULL, NULL);
			if (client_fd != INVALID_SOCKET)
			{
				printf("client connect\n");
				Client client;
				client.fd = client_fd;
				client.data = 0;
				client.connected = true;
				auto it = find_if(begin(client_list), end(client_list), [&](const Client& c)
				{
					return !c.connected;
				});
				if (it != end(client_list))
				{
					*it = client;
				}
				else
				{
					closesocket(client_fd);
				}
			}
		}
		for (auto& v : client_list)
		{
			if (v.connected)
			{
				if (FD_ISSET(v.fd, &rset))
				{
					char buf[128] = {0};
					int n = recv(v.fd, buf, 2, 0);
					if (n <= 0)
					{
						printf("client disconnect\n");
						auto it = find_if(begin(client_list), end(client_list), [&](const Client& c)
						{
							return c.fd == v.fd;
						});
						if (it != end(client_list))
						{
							it->connected = false;
						}
						closesocket(v.fd);
					}
					else
					{
						printf("recv %s\n", buf);
					}
				}
			}
			if (v.connected)
			{
				if (FD_ISSET(v.fd, &wset))
				{
					char buf[128] = {0};
					sprintf_s(buf, 128, "%d", v.data++);
					int n = send(v.fd, buf, 128, 0);
					if (n < 0)
					{
						printf("client disconnect\n");
						auto it = find_if(begin(client_list), end(client_list), [&](const Client& c)
						{
							return c.fd == v.fd;
						});
						if (it != end(client_list))
						{
							it->connected = false;
						}
						closesocket(v.fd);
					}
				}
			}
		}
	}
	return 0;
}
