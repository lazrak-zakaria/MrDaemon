#include <iostream>
#include <iomanip>
#include <ctime>
#include <vector>
#include "hpp/Daemon.hpp"




int main()
{
    Tintin_reporter reporter;
    DaemonServer daemon_server(&reporter);
    DaemonApp app(&reporter, &daemon_server);

    if (!app.init())
        return 1;
    return app.run();

}