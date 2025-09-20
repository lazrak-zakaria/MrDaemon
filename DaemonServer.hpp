
#ifndef DAEMONSERVER_HPP
#define DAEMONSERVER_HPP

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unordered_map>
#include "./Client.hpp"
#include "./Tintin_reporter.hpp"
#include <vector>

class DaemonServer
{

    private:
        std::unordered_map<int, Client> clients;
        int                             fdSock;
        struct sockaddr_in              addrServer;
        Tintin_reporter*                report_;

        DaemonServer();                    
        DaemonServer(const DaemonServer& other);     
        DaemonServer& operator=(const DaemonServer& other);
        void    acceptClient(fd_set &readSet);
        void    clear();

    public:
        bool	socketBindListen();
        bool    run();
        std::vector<std::string>  split(std::string &s, std::string delimiter);
        
        DaemonServer(Tintin_reporter* report_);
        ~DaemonServer();                               



};

#endif 