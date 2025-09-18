
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
    public:
        Client();                              
        Client(const Client& other);
        Client& operator=(const Client& other);
        ~Client();
        
        std::string data;


};

#endif