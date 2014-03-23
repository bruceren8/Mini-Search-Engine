#ifndef _TCP_SOCKET_
#define _TCP_SOCKET_

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <dirent.h>

using namespace std;

int tcp_init(const char * ip, unsigned short port);

int tcp_accept(int sfd);

int tcp_connect(const char *ip, unsigned short port);


#endif
