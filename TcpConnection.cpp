#include "TcpConnection.h"
#include "stdio.h"
TcpConnection::TcpConnection()
	: m_cs(CS_DISCONNETED)
	, m_fd(INVALID_SOCKET)
{
	m_initialized = false;
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	if (WSAStartup(version, &wsaData) == 0)
	{
		if (LOBYTE(wsaData.wVersion) == 2 && HIBYTE(wsaData.wVersion) == 2)
		{
			m_initialized = true;
		}
	}
}

TcpConnection::~TcpConnection()
{
	if (m_initialized)
	{
		WSACleanup();
	}
}

void TcpConnection::OnConnectedFailed()
{
	printf("connect failed\n");
}

void TcpConnection::OnConnectSuccess()
{
	printf("connect success\n");
}

void TcpConnection::OnDisconnected()
{
	printf("disconnected\n");
}

void TcpConnection::Connect(const char* ip, const int port)
{
	if (!m_initialized)
	{
		OnConnectedFailed();
		return;
	}
	m_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_fd == INVALID_SOCKET)
	{
		OnConnectedFailed();
		return;
	}
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);
	
	u_long block = 1;
	if (ioctlsocket(m_fd, FIONBIO, &block) == SOCKET_ERROR)
	{
		OnConnectedFailed();
		return;
	}
	int ret = connect(m_fd, (sockaddr*)&addr, sizeof(sockaddr));
	if (ret == 0)
	{
		m_cs = CS_CONNECTED;
		OnConnectSuccess();
		return;
	}
	if (WSAGetLastError() != WSAEWOULDBLOCK)
	{
		OnConnectedFailed();
		return;
	}
	m_cs = CS_CONNETING;
	printf("connecting\n");
}

bool TcpConnection::Initialize()
{
	if (!m_initialized)
	{
		WSADATA wsaData;
		WORD version = MAKEWORD(2, 2);
		if (WSAStartup(version, &wsaData) != 0)
		{
			return false;
		}
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		{
			return false;
		}
		m_initialized = true;
	}
	return m_initialized;
}

void TcpConnection::Update()
{
	if (m_cs == CS_CONNECTED)
	{
		fd_set rset;
		FD_ZERO(&rset);
		FD_SET(m_fd, &rset);
		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 1;
		int ret = select(0, &rset, NULL, NULL, &tv);
		if (ret < 0)
		{
			closesocket(m_fd);
			m_fd = INVALID_SOCKET;
			m_cs = CS_DISCONNETED;
			OnDisconnected();
		}
		else if (ret > 0)
		{
			if (FD_ISSET(m_fd, &rset))
			{
				char buf[128] = {0};
				int n = recv(m_fd, buf, 128, 0);
				if (n == 0)
				{
					int close = 0;
				}
				else if (n < 0)
				{
					int nn = -1;
					closesocket(m_fd);
					m_fd = INVALID_SOCKET;
					m_cs = CS_DISCONNETED;
					OnDisconnected();
				}
				else
				{
					int nn = 1;
				}
			}
		}
		else
		{
			int a = 0;
		}
	}
	else if (m_cs == CS_CONNETING)
	{
		fd_set wset;
		fd_set eset;
		FD_ZERO(&wset);
		FD_ZERO(&eset);
		FD_SET(m_fd, &wset);
		FD_SET(m_fd, &eset);

		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 1;
		int ret = select(0, NULL, &wset, &eset, &tv);
		if (ret < 0)
		{
			closesocket(m_fd);
			m_fd = INVALID_SOCKET;
			m_cs = CS_DISCONNETED;
			OnDisconnected();
		}
		else if (ret > 0)
		{
			if (FD_ISSET(m_fd, &eset))
			{
				m_fd = INVALID_SOCKET;
				m_cs = CS_DISCONNETED;
				OnConnectedFailed();
			}
			if (FD_ISSET(m_fd, &wset))
			{
				m_cs = CS_CONNECTED;
				OnConnectSuccess();
			}
		}
		else
		{
			int a = 0;
		}
	}
}
