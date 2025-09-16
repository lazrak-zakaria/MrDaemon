#include "Daemon.hpp"
#include <iostream>

int main()
{
    DaemonApp app;
    if (!app.init())
    {
        printf("hello");
    }
    printf("world");
    // return app.run();
}