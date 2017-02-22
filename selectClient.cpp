/*
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <cstring>
#include <string>
#include <stdlib.h>
*/
#include "selectClient.h"

using namespace std;

const int MAXBUFSIZE = 1024;

int main(int argc, char **argv)
{
	if(argc < 3)
		err_sys("usage: ./selectClient <IP> <Port>"); 

	const char *ip = argv[1];
	int port = atoi(argv[2]);

	clientSocket Socket = clientSocket(ip, port);
	int sockfd = Socket.getSockfd();
	if(sockfd < 0)
		err_sys("Socket error"); 
	
	int ret = Socket.connectServer();
	if(ret < 0)
		err_sys("connect error"); 

	char buffer[MAXBUFSIZE];
	fd_set rset;
	int stdineof, n;

	stdineof = 0;
	while(1)
	{
		FD_ZERO(&rset);
		FD_SET(sockfd, &rset);
		if(stdineof == 0)
			FD_SET(0, &rset);
		
		ret = select(sockfd + 1, &rset, NULL, NULL, NULL);
		
		if(FD_ISSET(sockfd, &rset))
		{
			memset(buffer, 0, sizeof(buffer));

			n = recv(sockfd, buffer, sizeof(buffer), 0);
			if(n == 0)
			{
				if(stdineof == 1)
					break;
				else
					err_sys("server terminated prematurely"); 
			}
			
			//这里不使用n的原因是：服务端在 send “进入...” "离开..." 
			//等由于使用中文所以发送的字节数多于实际数据
			write(0, buffer, strlen(buffer));
			cout << endl;
		}

		if(FD_ISSET(0, &rset))
		{
			memset(buffer, 0, sizeof(buffer));
			if( (n = read(0, buffer, sizeof(buffer))) == 0)
			{
				stdineof = 1;
				shutdown(sockfd, SHUT_WR);
				FD_CLR(0, &rset);
				continue;
			}
			if(buffer[n-1] == '\n')
				buffer[n-1] = '\0';

			if(strncmp(buffer, "quit", 4) == 0)
				break;
			
			write(sockfd, buffer, strlen(buffer));
		}
	}
	
	return 0;
}

