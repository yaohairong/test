#pragma once
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

class TcpConnection
{
private:
	enum ConnectState
	{
		CS_DISCONNETED,
		CS_CONNETING,
		CS_CONNECTED,
	};

public:
	TcpConnection();

	~TcpConnection();

	void Connect(const char* ip, const int port);

	virtual void OnConnectedFailed();

	virtual void OnConnectSuccess();

	virtual void OnDisconnected();

	bool Initialize();

	void Update();

	bool IsConnected()
	{
		return m_cs == CS_CONNECTED;
	}
	
	bool IsDisconnected()
	{
		return m_cs == CS_DISCONNETED;
	}

	bool IsConnecting()
	{
		return m_cs == CS_CONNETING;
	}

private:
	ConnectState m_cs;
	bool m_initialized;
	SOCKET m_fd;
	bool m_can_writable;
	bool m_can_readable;
};
