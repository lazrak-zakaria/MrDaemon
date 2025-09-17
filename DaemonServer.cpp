
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include "./DaemonServer.hpp"
#include <vector>
#include <fstream> 
#include <unistd.h>


std::vector<std::string> split(std::string &s, std::string delimiter) {
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


void	DaemonServer::socketBindListen()
{
	fdSock = socket(AF_INET, SOCK_STREAM, 0);
    if (fdSock == -1)
    {
        perror("socket");
        exit(1);
    }
    bzero(&addrServer, sizeof(addrServer));
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = INADDR_ANY;
    addrServer.sin_port = htons(4252);
	int optval = 1;
	setsockopt(fdSock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));
    if (bind(fdSock, (struct sockaddr *) &addrServer,
              sizeof(addrServer)) < 0) 
    {
        perror("bind");
        exit(1);
    }
    if (listen(fdSock, SOMAXCONN))
	{
		perror("listen");
        exit(1);
	}
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
		std::cerr << "connetion error\n";
        return ;
	}

	FD_SET(fdSockTmp, &readSet);
    clients[fdSockTmp] = Client();
    
}

void    DaemonServer::run()
{
    fd_set              				readSet;
    fd_set              				writeSet;

    FD_ZERO(&readSet);

    this->socketBindListen();
    
    FD_SET(fdSock, &readSet);

    std::ofstream file("logger.txt", std::ios::out); // logger wont be here
    if (!file.is_open()) {
        perror("file");
        std::cerr << "Error opening file!" << std::endl;
        return;
    }
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
            std::cout<<"exit\n";
            continue;
        }
        else if (selectAnswer)
        {
            if (FD_ISSET(fdSock, &tempReadSet))
			{
                if (clients.size() <= 3)
                    acceptClient(readSet);
                else
                    std::cerr << "user 4 tried to connect" << "\n" ;//loger
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
                        continue ;
                    }
                    it.second.data.append(buf, collected);

                    split(it.second.data, "\n");
                    for (auto & i : it.second.data)
                        file << i << std::endl;
                }
            }

            for (auto &fd : invalidSockets){
                close(fd);
                clients.erase(fd);
            }
        }
    }
    
}


