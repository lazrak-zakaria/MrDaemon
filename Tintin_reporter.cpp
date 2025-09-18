#include "Tintin_reporter.hpp"
#include <sys/stat.h>
#include <ctime>

Tintin_reporter::Tintin_reporter(){};
Tintin_reporter::~Tintin_reporter()
{
    if (Info_.is_open()) Info_.close();
    if (Error_.is_open()) Error_.close();
    if (Log_.is_open()) Log_.close();
};

bool Tintin_reporter::create_dir()
{
     struct stat st;
    printf("++++++++");
    if (stat("/var/log/matt_daemon", &st) == 0)
    {
        if (S_ISDIR(st.st_mode) == 0)
        {
            printf("dsssss");
            return true;
        }
        return false;
    }
    if (mkdir("/var/log/matt_daemon", 0755) == -1)
    {
        printf("llllll|");
        return false;
    }
    return true;
}

bool Tintin_reporter::init()
{
    if (!create_dir())
        return true;
    Info_.open("/var/log/matt_daemon/Info.log", std::ios::out | std::ios::app);
    Error_.open("/var/log/matt_daemon/Error.log", std::ios::out | std::ios::app);
    Log_.open("/var/log/matt_daemon/Log.log", std::ios::out | std::ios::app);

    if (!Info_ || !Error_ || !Log_)
    {
        printf("dddd");
        return true;
    }
    return false;
};

std::string Tintin_reporter::getTimestamp()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "[%d/%m/%Y - %H:%M:%S]", &tstruct);
    return buf;
};

std::string Tintin_reporter::levelToString(level level)
{
    switch(level) {
        case INFO:  return "[ INFO ]";
        case ERROR: return "[ ERROR ]";
        case LOG:   return "[ LOG ]";
    }
    return "[ UNKNOWN ]";
};

void Tintin_reporter::log(level level, std::string message)
{
    std::string entry = getTimestamp() + " " + levelToString(level) + " - Matt_daemon: " + message + "\n";

        switch(level) {
            case INFO:  Info_ << entry;  Info_.flush(); break;
            case ERROR: Error_ << entry; Error_.flush(); break;
            case LOG:   Log_ << entry;   Log_.flush(); break;
        }
};


void Tintin_reporter::send_mail(std::string msg)
{
    std::ofstream email("email.txt");
    email << "From: Joseph <josephardev@gmail.com>\n";
    email << "To: Recipient <recipient@example.com>\n";
    email << "Subject: Test Email from C++\n\n";
    email << msg;
    email.close();

    std::string cmd = "curl --url 'smtps://smtp.gmail.com:465' "
                      "--ssl-reqd "
                      "--mail-from 'josephardev@gmail.com' "
                      "--mail-rcpt 'recipient@example.com' "
                      "--user 'josephardev@gmail.com:YOUR_APP_PASSWORD' "
                      "--upload-file email.txt";

    system(cmd.c_str());
    std::remove("email.txt");
};