/*
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cstring>
*/
#include "selectServer.h"

using namespace std;

int clientNum = 10;
const int MAXBUFFSIZE = 1024;

typedef struct clientData_
{
	int fd;
	struct sockaddr_in addr;
}clientData;


string getAddrInfo(clientData client, int currentClientNum, int flag);
void sendMsgToOthers(clientData *client, int self, const char *buff);
void handlerMessage(clientData *client, int currentIndex, char *buff, fd_set &readySet, int &currentClientNum);


int main(int argc, char **argv)
{
	if(argc != 4)
		err_sys("usage: /selectServer <IP> <Port> <maxClientNum>");

	const char *servIP = argv[1];
	int servPort = atoi(argv[2]);
    clientNum = atoi(argv[3]);

	serverSocket Socket = serverSocket(servIP, servPort);
	int listenfd = Socket.getListenfd();

	fd_set rset, allset;
	char buffer[MAXBUFFSIZE];
	int currentClientNum = 0;
	int maxfd = listenfd;
	int nready, i;
	string clientInfo;

	clientData client[clientNum];
	for(i = 0; i < clientNum; ++i)
		client[i].fd = -1;

	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	while(1)
	{
		rset = allset;
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		if(nready < 0)
			err_sys("select init error");

		if(FD_ISSET(listenfd, &rset))
		{
			struct sockaddr_in cliAddr;
			socklen_t cliAddrLen = sizeof(cliAddr);

			int connfd = Socket.Accept(listenfd, (struct sockaddr*)&cliAddr, &cliAddrLen);	
			if(connfd < 0)
				err_sys("Accept error");
			else	//处理新连接
			{
				if(currentClientNum == clientNum)
				{
					cout << "聊天室已满!" << endl;
					send(connfd, "聊天室已满!", 30, 0);
					close(connfd);	//人已满则断开新加入的连接
					continue;
				}

				for(i = 0; i < clientNum; ++i)
				{
					if(client[i].fd == -1){
						client[i].fd = connfd;
						client[i].addr = cliAddr;
						++currentClientNum;

						FD_SET(connfd, &allset);
						if(maxfd < connfd)
							maxfd = connfd;

						clientInfo = getAddrInfo(client[i], currentClientNum, 1);
						cout << clientInfo << endl;
						sendMsgToOthers(client, i, clientInfo.c_str());
						clientInfo.clear();
						send(connfd, "          欢迎进入聊天室!\n", 50, 0);
						break;
					}
				}
			}
			if(--nready <= 0)	//根据select返回活动描述符数量判断是否需要进行下面对客户端的轮询
				continue;
		}
		
		//处理已连接的客户端
		for(i = 0; i < clientNum; ++i)
		{
			if(client[i].fd < 0)
				continue;
			
			if(FD_ISSET(client[i].fd, &rset))
			{
				memset(buffer, 0, sizeof(buffer));
				handlerMessage(client, i, buffer, allset, currentClientNum);
				if(--nready <= 0)
					continue;
			}
		}
	}

	return 0;
}


string getAddrInfo(clientData client, int currentClientNum, int flag)
{
	string clientInfo;
	
	clientInfo.append(inet_ntoa(client.addr.sin_addr));
	clientInfo.append(" : ");
	string portNum = to_string(ntohs(client.addr.sin_port));
	clientInfo.append(portNum);

	switch(flag)
	{
	case 0:
		clientInfo.append(" ---> "); return clientInfo;
	case 1:
		clientInfo.append(" 进入聊天室! "); break;
	case 2:
		clientInfo.append(" 离开聊天室! "); break;
	default: break;
	}
	
	clientInfo.append("当前人数为: ");
	string personNum = to_string(currentClientNum);
	clientInfo.append(personNum);

	return clientInfo;
}


void sendMsgToOthers(clientData *client, int self, const char *buff)
{
	for(int i = 0; i < clientNum; ++i)
		if( (client[i].fd != -1) && (i != self) )
			send(client[i].fd, buff, strlen(buff), 0);
}


void handlerMessage(clientData *client, int currentIndex, char *buff, fd_set &aset, int &currentClientNum)
{
	string clientMesg;

	int ret = recv(client[currentIndex].fd, buff, MAXBUFFSIZE, 0);
	if(ret <= 0)	//客户端关闭，或接收到 RST
	{
		close(client[currentIndex].fd);
		FD_CLR(client[currentIndex].fd, &aset);
		client[currentIndex].fd = -1;
		--currentClientNum;

		clientMesg = getAddrInfo(client[currentIndex], currentClientNum, 2);
		cout << clientMesg << endl;
		sendMsgToOthers(client, currentIndex, clientMesg.c_str());
	}
	else	//处理客户端数据
	{
		if(buff[ret-1] == '\n')
			buff[ret-1] = '\0';

		clientMesg = getAddrInfo(client[currentIndex], currentClientNum, 0);
		cout << clientMesg;
		cout << buff << endl;
	
		sendMsgToOthers(client, currentIndex, clientMesg.c_str());
		sendMsgToOthers(client, currentIndex, buff);
	}
}

