
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unordered_map>
#include "./Client.hpp"
#include "./Tintin_reporter.hpp"
class DaemonServer
{

    private:
        std::unordered_map<int, Client> clients;
        int                             fdSock;
        struct sockaddr_in              addrServer;
        Tintin_reporter*                report_;
    public:
        bool	socketBindListen();
        void    acceptClient(fd_set &readSet);
        bool    run();
        DaemonServer(Tintin_reporter* report_);

};