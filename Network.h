#pragma once
#include <vector>

class Server;
class TcpStream
{
public:
	enum Error
	{
		ERR_READ,
		ERR_SEND,
		ERR_RECV,
		ERR_WRITE,
	};
	TcpStream();

	~TcpStream();

	void BeginWrite()
	{
		send_buf_len = 2;
	}

	void WriteInt(int data);

	void WriteStr(char* data, int len);

	void EndWrite();

	void ReadInt(int& data);

	virtual void OnConnected();

	virtual void OnDisconnected();

	virtual void OnMessage();

private:
	void Recv();

	//void Send();

	void Close();

	void Clear();

private:
	static const int RECV_BUF_SIZE = 1024;
	static const int SEND_BUF_SIZE = 1024;
	int m_sock_fd;
	
	static char send_buf[SEND_BUF_SIZE];
	static int send_buf_len;

	char m_recv_buf[RECV_BUF_SIZE];
	int m_recv_buf_len;
	bool m_readable;
	int m_read_offset;

	friend class Server;
};

class Server
{
public:
	Server();

	~Server();

	void Start(short listen_port);

	void AddClient(TcpStream* client);

private:
	void OnAccept();

	void Loop();

private:
	int m_sock_fd;
	int m_epoll_fd;
	std::vector<TcpStream*> m_client_list;
};

class Client : public TcpStream
{
public:
	Client();

	~Client();

	void OnConnected() override;

	void OnDisconnected() override;

	void OnMessage() override;

	void ResponseLogin();
};
