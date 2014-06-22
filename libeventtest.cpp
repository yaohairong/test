#include <stdlib.h>
#include <stdio.h>
#include <event.h>
#include <string.h>
#include <arpa/inet.h>

void read_cb(bufferevent* be, void* ptr)
{
}

void write_cb(bufferevent* be, void* ptr)
{
}

void event_cb(bufferevent* be, short events, void* ptr)
{
	if (events & BEV_EVENT_CONNECTED)
	{
		printf("connect success\n");
	}
	else if (events & BEV_EVENT_ERROR)
	{
		printf("event error\n");
	}
}

int main (int argc, char **argv)
{
	event_base* eb = event_base_new();
	bufferevent *be = bufferevent_socket_new(eb, -1, BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(be, read_cb, write_cb, event_cb, NULL);
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("192.168.1.100");
	addr.sin_port = htons(7001);

	if (bufferevent_socket_connect(be, (sockaddr*)&addr, sizeof(addr)) < 0)
	{
		printf("connect error\n");
		bufferevent_free(be);
		return -1;
	}
	bufferevent_enable(be, EV_READ);
	event_base_dispatch(eb);
	return 0;
}

