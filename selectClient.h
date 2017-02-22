#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <string>

using namespace std;

class clientSocket
{
private:
	int sockfd;
	struct sockaddr_in remoteAddr;
	struct sockaddr_in localAddr;

public:
	clientSocket(const char *ip, int port);
	~clientSocket();
	int connectServer();
	int getSockfd() const;
};

void err_sys(const char *perrno)
{
	cerr << *perrno << endl;
	exit(1);
}


clientSocket::clientSocket(const char *ip, int port)
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
		err_sys("socket error"); 

	bzero(&remoteAddr, sizeof(remoteAddr));
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &remoteAddr.sin_addr);
}

clientSocket::~clientSocket()
{
	if(sockfd > 0)
		close(sockfd);
}

int clientSocket::connectServer()
{
	int ret = connect(sockfd, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr));
	if(ret < 0)
		err_sys("connect error"); 		

	return ret;
}

int clientSocket::getSockfd() const
{
	return sockfd;
}

