#include "tcp_socket.h"

int tcp_init(const char * ip, unsigned short port)
{
	//socket
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
	{
		perror("socket");
		exit(-1);
	}
	
	//设置端口可立即重用
	int reuse=1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(int));

	//bind
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(struct sockaddr_in));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr(ip);
	if (bind(sfd, (struct sockaddr*)&saddr, sizeof(struct sockaddr)) == -1)
	{
		perror("bind");
		close(sfd);
		exit(-1);
	}

	//listen
	if (listen(sfd, 10) == -1)
	{
		perror("listen");
		close(sfd);
		exit(-1);
	}
	return sfd;
}

int tcp_accept(int sfd)
{
	//accept
	struct sockaddr_in caddr;
	memset(&caddr, 0, sizeof(struct sockaddr_in));
	socklen_t iLen = sizeof(struct sockaddr_in);
	int new_fd = accept(sfd, (struct sockaddr*)&caddr, &iLen);
	if (new_fd == -1)
	{
		perror("accept");
		close(sfd);
		exit(-1);
	}
	cout <<  inet_ntoa(caddr.sin_addr) << "[" <<  ntohs(caddr.sin_port) << "]";
	cout << "has connected successed!" << endl;

	return new_fd;
	
}

int tcp_connect(const char *ip, unsigned short port)
{
	//socket
	int sfd = socket(AF_INET, SOCK_STREAM, 0);

	//设置端口可立即重用
	int reuse=1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(int));

	if (sfd == -1)
	{
		perror("socket");
		exit(-1);
	}

	//connect
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(struct sockaddr_in));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr(ip);
	if (connect(sfd, (struct sockaddr*)&saddr, sizeof(struct sockaddr)) == -1)
	{
		perror("connect");
		close(sfd);
		exit(-1);
	}
	return sfd;
}

