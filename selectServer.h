#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <string>

using namespace std;

class serverSocket
{
private:
	int listenfd;
	struct sockaddr_in localAddr;

public:
	serverSocket(const char *ip, int port);
	~serverSocket();
	int Accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);
	struct sockaddr_in getRemoteAddr() const;
	int getListenfd() const;
};

void err_sys(const char *perrno)
{
	cout << *perrno << endl;
	exit(1);
}

serverSocket::serverSocket(const char *ip, int port)
{
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0)
		err_sys("socket error");

	bzero(&localAddr, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &localAddr.sin_addr);
	
	int ret = bind(listenfd, (struct sockaddr*)&localAddr, sizeof(localAddr));
	if(ret < 0)
		err_sys("bind error");

	ret = listen(listenfd, 5);
	if(ret < 0)
		err_sys("listen error");
}

serverSocket::~serverSocket()
{
	if(listenfd > 0)
		close(listenfd);
}

int serverSocket::Accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen)
{
	int connfd = accept(sockfd, cliaddr, addrlen);
	if(connfd < 0)
		err_sys("accept error");

	return connfd;
}

int serverSocket::getListenfd() const
{
	return listenfd;
}

