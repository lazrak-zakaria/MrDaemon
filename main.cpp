#include "Daemon.hpp"
#include <iostream>


#include <iostream>
#include <iomanip>
#include <ctime>
#include <vector>

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

int main() {
    // std::time_t t = std::time(nullptr);   // get current time
    // std::tm* now = std::localtime(&t);    // convert to local time
    
    // std::cout << "["
    //           << std::setfill('0') << std::setw(2) << now->tm_mday << "/"
    //           << std::setfill('0') << std::setw(2) << now->tm_mon + 1 << "/"
    //           << now->tm_year + 1900 << "-"
    //           << std::setfill('0') << std::setw(2) << now->tm_hour << ":"
    //           << std::setfill('0') << std::setw(2) << now->tm_min << ":"
    //           << std::setfill('0') << std::setw(2) << now->tm_sec
    //           << "]\n";
    std::string v = "fasdfads\nadfsdf\nfinal";
    std::vector<std::string> k = split(v , "\n");
    for (auto & i : k)
        std::cout<< i;

    return 0;
}


// int main()
// {
//     // DaemonApp app;
//     // if (!app.init())
//     // {
//     //     printf("hello");
//     // }
//     // printf("world");
//     // return app.run();

//     print( "sadAS", 2,4,"dfass", 3.5, 'a');
// }