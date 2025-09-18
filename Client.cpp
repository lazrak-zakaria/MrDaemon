#include "Client.hpp"

Client::Client() : data("")
{
}

Client::Client(const Client& other) : data(other.data)
{
}

Client& Client::operator=(const Client& other)
{
    if (this != &other)
    {
        data = other.data;
    }
    return *this;
}

Client::~Client()
{
}

