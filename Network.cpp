#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Network.h"
#include <algorithm>
bool SetNonblocking(int sock_fd)
{
	int opt = fcntl(sock_fd, F_GETFL);
	if (opt < 0)
	{
		return false;
	}
	return fcntl(sock_fd, F_SETFL, opt | O_NONBLOCK) == 0;
}

char TcpStream::send_buf[SEND_BUF_SIZE] = {0};
int TcpStream::send_buf_len = 2;

void TcpStream::ReadInt(int& data)
{
	if (m_read_offset + sizeof(int) > m_recv_buf_len)
	{
		throw ERR_READ;
	}
	data = ntohl(*(int*)(m_recv_buf + m_read_offset));
	m_read_offset += sizeof(int);
}

void TcpStream::OnConnected()
{
	printf("client connected\n");
}

void TcpStream::OnDisconnected()
{
	printf("client disconnected\n");
}

void TcpStream::OnMessage()
{

}

void TcpStream::WriteInt(int data)
{
	if (send_buf_len + sizeof(int) > SEND_BUF_SIZE)
	{
		throw ERR_WRITE;
	}
	*((int*)(send_buf + send_buf_len)) = htonl(data);

	send_buf_len += sizeof(int);
}

void TcpStream::WriteStr(char* data, int len)
{
	if (send_buf_len + len > SEND_BUF_SIZE)
	{
		throw ERR_WRITE;
	}
	memcpy(send_buf + send_buf_len, data, len);
	send_buf_len += len;
}

void TcpStream::EndWrite()
{
	printf("send buf len %d\n", send_buf_len);
	*((short*)(send_buf)) = htons(send_buf_len);
	int n = send(m_sock_fd, send_buf, send_buf_len, 0);
	if (n < send_buf_len)
	{
		if (errno == EINTR)
		{
			EndWrite();
		}
		else
		{
			throw ERR_SEND;
		}
	}
}

void TcpStream::Recv()
{
	while (m_recv_buf_len < RECV_BUF_SIZE && m_readable)
	{
		int n = recv(m_sock_fd, m_recv_buf + m_recv_buf_len, RECV_BUF_SIZE - m_recv_buf_len, 0);
		if (n > 0)
		{
			m_recv_buf_len += n;
		}
		else if (n == 0)
		{
			printf("recv 0\n");
			Close();
			break;
		}
		else
		{
			if (errno == EWOULDBLOCK)
			{
				m_readable = false;
				break;
			}
			else if (errno != EINTR)
			{
				perror("recv err\n");
				Close();
				break;
			}
		}
	}
	if (m_recv_buf_len > 2)
	{
		int msg_len = ntohs(*((short*)(m_recv_buf)));
		if (m_recv_buf_len >= msg_len)
		{
			m_read_offset += 2;
			try
			{
				OnMessage();
			}
			catch (...)
			{
				printf("catch ...\n");
				Close();
			}
		}
	}
}

//void TcpStream::Send()
//{
//	while (m_send_buf_len > 0 && m_writeable)
//	{
//		int n = send(m_sock_fd, m_send_buf, m_recv_buf_len, 0);
//		if (n > 0)
//		{
//			memcpy(m_send_buf, m_send_buf + n, m_send_buf_len - n);
//			m_send_buf_len -= n;
//			continue;
//		}
//		else
//		{
//			if (errno == EWOULDBLOCK)
//			{
//				m_writeable = false;
//			}
//			else if (errno != EINTR)
//			{
//
//				Close();
//			}
//		}
//	}
//}

TcpStream::TcpStream()
{
	Clear();
}

TcpStream::~TcpStream()
{
	if (m_sock_fd != -1)
	{
		Clear();
	}
}

void TcpStream::Close()
{
	if (m_sock_fd > 0)
	{
		close(m_sock_fd);
		Clear();
		OnDisconnected();
	}
	//if (close(m_sock_fd) < 0)
	//{
	//	perror("close");
	//}
	//Clear();
	//OnDisconnected();
}

void TcpStream::Clear()
{
	m_sock_fd = -1;
	memset(m_recv_buf, 0, RECV_BUF_SIZE);
	m_recv_buf_len = 0;
	m_readable = false;
	m_read_offset = 0;
}

Server::Server()
	: m_sock_fd(-1)
	, m_epoll_fd(-1)
{

}

Server::~Server()
{

}

void Server::Start(short listen_port)
{
	m_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock_fd < 0)
	{
		return;
	}
	int opt = 1;
	if (setsockopt(m_sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		return;
	}
	if (!SetNonblocking(m_sock_fd))
	{
		return;
	}
	sockaddr_in addr;
	bzero(&addr, sizeof(sockaddr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(listen_port);
	if (bind(m_sock_fd, (sockaddr*)&addr, sizeof(sockaddr)) < 0)
	{
		return;
	}
	if (listen(m_sock_fd, 20) < 0)
	{
		return;
	}
	m_epoll_fd = epoll_create(20);
	if (m_epoll_fd < 0)
	{
		return;
	}
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = m_sock_fd;
	if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_sock_fd, &ev) < 0)
	{
		return;
	}
	Loop();
}

void Server::AddClient(TcpStream* client)
{
	m_client_list.push_back(client);
}

void Server::Loop()
{
	epoll_event events[20];
	while (true)
	{
		int n = epoll_wait(m_epoll_fd, events, 20, -1);
		if (n > 0 )
		{
			for (int i = 0; i < n; i++)
			{
				epoll_event* ev = &events[i];
				if (ev->data.fd == m_sock_fd)
				{
					OnAccept();
				}
				else
				{
					TcpStream* client = (TcpStream*)ev->data.ptr;
					if (ev->events & EPOLLIN)
					{
						client->m_readable = true;
						client->Recv();
					}
					if (ev->events & EPOLLERR)
					{
						perror("epoll err");
						client->Close();
					}
					else if (ev->events & EPOLLHUP)
					{
						perror("epoll hup");
						client->Close();
					}
					//if (ev->events & EPOLLOUT)
					//{
					//	TcpStream* client = (TcpStream*)ev->data.ptr;
					//	client->m_writeable = true;
					//	client->Send();
					//}

				}
			}
		}
		else if (errno != EINTR)
		{
			exit(1);
		}
	}
}

void Server::OnAccept()
{
	while (true)
	{
		sockaddr_in addr;
		socklen_t addr_len = sizeof(sockaddr_in);
		int fd = accept(m_sock_fd, (sockaddr*)&addr, &addr_len);
		if (fd < 0)
		{
			if (errno == EAGAIN)
			{
				break;
			}
			else if (errno == EINTR)
			{
				continue;
			}
			else
			{
				exit(1);
			}
		}
		else
		{
			auto find_it = find_if(begin(m_client_list), end(m_client_list), [&](const TcpStream* client)
			{
				return client->m_sock_fd == -1;
			});

			if (find_it != end(m_client_list))
			{
				if (!SetNonblocking(fd))
				{
					exit(1);
				}
				epoll_event ev;
				ev.data.ptr = *find_it;
				ev.events = EPOLLIN | EPOLLET;
				//ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
				if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0)
				{
					exit(1);
				}
				(*find_it)->Clear();
				(*find_it)->m_sock_fd = fd;
				(*find_it)->OnConnected();
			}
			else
			{
				close(fd);
			}
		}
	}
}

Client::Client()
{

}

Client::~Client()
{

}

void Client::OnConnected()
{
	printf("client connected\n");
}

void Client::OnDisconnected()
{
	printf("client disconnected\n");
}

void Client::OnMessage()
{
	printf("on message\n");
	int cmd;
	ReadInt(cmd);
	printf("cmd %d\n", cmd);
	switch (cmd)
	{
	case 1002:
		ResponseLogin();
		break;
	case 1015:
		break;
	default:
		printf("unkown message\n");
		break;
	}
}

void Client::ResponseLogin()
{
	BeginWrite();
	WriteInt(1001);
	WriteInt(0);
	char buf[64] = {0};
	snprintf(buf, 64, "hello yhr");
	WriteStr(buf, 64);
	EndWrite();
	BeginWrite();
	WriteInt(1004);
	WriteInt(0);
	WriteInt(1);
	EndWrite();
	BeginWrite();
	WriteInt(1006);
	WriteInt(0);
	snprintf(buf, 32, "yhr");
	WriteStr(buf, 32);
	int id = 314;
	WriteInt(id);
	int vip_rank = 0;
	WriteInt(vip_rank);
	int vip_exp = 0;
	WriteInt(vip_exp);
	int icon = 2;
	WriteInt(icon);
	int rank = 0;
	WriteInt(rank);
	int exp = 0;
	WriteInt(exp);
	int last_update = 0;
	WriteInt(last_update);
	int ac_point = 100;
	WriteInt(ac_point);
	int server_time = 0;
	WriteInt(server_time);
	char skill[129] = {0};
	WriteStr(skill, 129);
	int arena_rank = 123;
	WriteInt(arena_rank);
	int score = 34;
	WriteInt(score);
	int sign_count = 0;
	WriteInt(sign_count);
	int sign_status = 0;
	WriteInt(sign_status);
	int login_count = 0;
	WriteInt(login_count);
	EndWrite();
}
