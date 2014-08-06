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

int main()
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		perror("socket");
		exit(1);
	}
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		perror("set sock opt");
		exit(1);
	}
	sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(6314);

	if (bind(server_fd, (sockaddr*)&server_addr, sizeof(sockaddr)) < 0)
	{
		perror("bind");
		exit(1);
	}

	if (listen(server_fd, 20) < 0)
	{
		perror("listen");
		exit(1);
	}
	sockaddr_in client_addr;
	bzero(&client_addr, sizeof(client_addr));
	socklen_t addr_len;
	int client_fd = accept(server_fd, (sockaddr*)&client_addr, &addr_len);
	if (client_fd < 0)
	{
		perror("accept");
		exit(1);
	}
	int pid = vfork();
	if (pid < 0)
	{
		perror("vfork");
		exit(1);
	}
	else if (pid == 0)
	{
		fprintf(stdout, "child\n");
	}
	else
	{
		fprintf(stdout, "parent\n");	
	}
	return 0;
}
