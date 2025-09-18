#include "Daemon.hpp"
#include <iostream>


#include <iostream>
#include <iomanip>
#include <ctime>
#include <vector>




int main()
{
    Tintin_reporter reporter;
    // reporter.init();
    DaemonApp app(&reporter);
    if (!app.init())
    {
        printf("hello");
        return 1;
    }
    printf("world");
    return app.run();

}