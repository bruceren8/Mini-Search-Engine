#include "searchSys.h"
#include "tcp_socket.h"
#include <pthread.h>

const char* CONF_PATH = "../conf/searchSys.ini";
SearchSys sys(CONF_PATH);

void sendStr(int fd, string &str)
{
	/*
	int len[1];
	len[0] = str.size();
	send(fd, len, 4, 0);
	*/
	int cnt = 0;
	while (cnt != (int)str.size())
		cnt += send(fd, str.c_str() + cnt, str.size() - cnt, 0);
}

void *servFunc(void *arg)
{
	int sfd = (int)arg;
	fd_set readfds, testfds;
	FD_ZERO(&readfds);
	FD_SET(sfd, &readfds);
	while (1)
	{
		int fd;
		testfds = readfds;
		cout << "server waiting..." << endl;
		if (select(FD_SETSIZE, &testfds, NULL, NULL, NULL) < 1)
		{
			perror("select");
			exit(1);
		}
		for (fd = 0; fd < FD_SETSIZE; ++fd)
		{
			if (FD_ISSET(fd, &testfds))
			{
				if (fd == sfd)//活动发生在服务器套接字，是一个新的连接请求
				{
					int cfd = tcp_accept(sfd);
					FD_SET(cfd, &readfds);
				}
				else//活动发生在客户端套接字，是客户的活动
				{
					char quest[64] = {'\0'};
					if (recv(fd, quest, sizeof(quest), 0) == -1)
					{
						perror("recv");
						exit(-1);
					}
					vector<string> quest_segs;
					stringSplit(quest, ':', quest_segs);
					string query = quest_segs[0];
					int tag = atoi(quest_segs[1].c_str());
					int classNum = atoi(quest_segs[2].c_str());
					
					if (tag == 1)
					{
						int classCnt = sys.getClassCnt(query);
						//int cnt[1];
						//cnt[0] = classCnt;
						char cnt[5] = {'\0'};
						sprintf(cnt, "%d", classCnt);
						send(fd, cnt, sizeof(cnt), 0);
					}
					string result = sys.search(query, tag, classNum);
					sendStr(fd, result);

					close(fd);
					FD_CLR(fd, &readfds);
					cout << "removing client on fd:" << fd <<  endl;
				}
				break;
			}
		}
	}
}

void Daemon()
{
	//const int MAXFD = 64;
	if (fork() != 0)
		exit(0);
	setsid();
	//chdir("/");
	umask(0);
	//for (int i = 0; i < MAXFD; i++)
	//	close(i);
}

int main(int args, char *argv[])
{
	Daemon();//成为守护进程
	
	cout << "init..." << endl;
	sys.init();
	cout << "init success!" << endl;

	int sfd = tcp_init("192.168.152.146", 8888);
	pthread_t pthid;
	pthread_create(&pthid, NULL, servFunc, (void*)sfd);
	pthread_join(pthid, (void**)NULL);

	return 0;
}
