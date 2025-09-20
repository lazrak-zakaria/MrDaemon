#ifndef TINTIN_REPORTRER_HPP
#define TINTIN_REPORTRER_HPP

#include <filesystem>
#include <string>
#include <fstream>
enum level {
    ERROR,
    LOG,
    INFO
};

class Tintin_reporter
{
    public:
        Tintin_reporter();
        ~Tintin_reporter();
        bool init();
        void log(level flog, std::string message);
        bool create_dir();
        std::string getTimestamp();
        std::string levelToString(level f);
        void send_mail(std::string msg);
        Tintin_reporter(const Tintin_reporter& other) = delete;
        Tintin_reporter& operator=(const Tintin_reporter& other) = delete;
    
        private:
            std::ofstream Info_;
            std::ofstream Error_;
            std::ofstream Log_;
};

#endif