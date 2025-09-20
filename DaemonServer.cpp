
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include "./DaemonServer.hpp"
#include <vector>
#include <fstream> 
#include <unistd.h>
#include "Tintin_reporter.hpp"
#include <errno.h>

DaemonServer::DaemonServer()
    : fdSock(-1), report_(nullptr)
{
}

DaemonServer::DaemonServer(const DaemonServer& other)
    : clients(other.clients),
      fdSock(other.fdSock),
      addrServer(other.addrServer),
      report_(other.report_)
{
}


DaemonServer& DaemonServer::operator=(const DaemonServer& other)
{
    if (this != &other)
    {
        clients    = other.clients;
        fdSock     = other.fdSock;
        addrServer = other.addrServer;
        report_    = other.report_;
    }
    return *this;
}

DaemonServer::~DaemonServer()
{
    clear();
}



DaemonServer::DaemonServer(Tintin_reporter* report_)
: fdSock(-1), report_(report_)
{
}

std::vector<std::string>  DaemonServer::split(std::string &s, std::string delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length() );
    }
    return tokens;
}

bool	DaemonServer::socketBindListen()
{

    report_->log(INFO, "Creating server.");

	fdSock = socket(AF_INET, SOCK_STREAM, 0);
    if (fdSock == -1)
    {
        char *error_message = strerror(errno);
        report_->log(ERROR, error_message);
        return 1;
    }
    bzero(&addrServer, sizeof(addrServer));
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = INADDR_ANY;
    addrServer.sin_port = htons(4231);
	int optval = 1;
	setsockopt(fdSock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));
    if (bind(fdSock, (struct sockaddr *) &addrServer,
              sizeof(addrServer)) < 0) 
    {
        char *error_message = strerror(errno);
        report_->log(ERROR, error_message);
        return 1;

    }
    if (listen(fdSock, SOMAXCONN))
	{
        char *error_message = strerror(errno);
        report_->log(ERROR, error_message);
        return 1;
	}
    report_->log(INFO, "Server created.");

    return 0;
}



void	DaemonServer::acceptClient(fd_set &readSet)
{

	struct sockaddr_in  clientSockaddr;
	socklen_t			clientSockaddrLength ;
	int					fdSockTmp;

	memset(&clientSockaddr, 0, sizeof(clientSockaddr));
	clientSockaddrLength = 0;
	fdSockTmp = accept(fdSock, (struct sockaddr*)&clientSockaddr,  &clientSockaddrLength);
	if (fdSockTmp == -1)
	{
        char *error_message = strerror(errno);
        report_->log(ERROR, error_message);
        return;
	}
    if (clients.size() >= 3)
    {
        close(fdSockTmp);
        report_->log(ERROR, "Connection limit reached. A maximum of 3 clients can connect at the same time.");

        return;
    }

	FD_SET(fdSockTmp, &readSet);
    clients[fdSockTmp] = Client();
    
}

bool    DaemonServer::run()
{
    fd_set              				readSet;
    fd_set              				writeSet;

    FD_ZERO(&readSet);

    
    FD_SET(fdSock, &readSet);

    while (true)
    {
        fd_set	tempReadSet = readSet;
        std::vector<int>    invalidSockets;


        int maxSocket = fdSock;
        for (auto &it : clients)
            maxSocket = std::max(it.first, maxSocket);
        
        int selectAnswer = select(maxSocket + 1, &tempReadSet, 0, 0, 0);
		if (selectAnswer == -1)
		{
            if (errno == EINTR) {
                return 1;
            }
            continue;
        }
        else if (selectAnswer)
        {
            if (FD_ISSET(fdSock, &tempReadSet))
			{
                acceptClient(readSet);
            }

            for (auto &it : clients)
            {
                int clientFdSock = it.first;
                if (FD_ISSET(clientFdSock, &tempReadSet))
                {

                    char	buf[8192];
                    int		collected = recv(clientFdSock, buf, 8192, 0);

                    if (collected == -1 || collected == 0)
                    {
                        invalidSockets.push_back(clientFdSock);
                        FD_CLR(clientFdSock, &readSet);
                        report_->log(INFO, "User Disconnected");
                        
                        continue ;
                    }
                    it.second.data.append(buf, collected);

                    std::vector<std::string> arr = split(it.second.data, "\n");
                    for (auto & data : arr)
                    {
                        if ("quit" == data)
                        {
                            return 1;
                        }
                        report_->log(LOG, "User input: " + data);
                    }
                }
            }

            for (auto &fd : invalidSockets){
                close(fd);
                clients.erase(fd);
            }
        }
    }
    return 0;
}

void     DaemonServer::clear()
{
        for (auto &it : clients)
        {
            int clientFdSock = it.first;
            close(clientFdSock);
        }
        close(fdSock);
}