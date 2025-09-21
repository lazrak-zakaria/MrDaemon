#include "../hpp/Tintin_reporter.hpp"
#include <sys/stat.h>
#include <ctime>
#include <unistd.h>

Tintin_reporter::Tintin_reporter()
{
    char* pass = std::getenv("PASS");
    char* to = std::getenv("TO");
    this->to = "josephardev@gmail.com";
    this->pass = "";
    if (pass)
        this->pass = pass;
    if (to)
        this->to = to;
};
Tintin_reporter::~Tintin_reporter()
{
    if (Info_.is_open()) Info_.close();
    if (Error_.is_open()) Error_.close();
    if (Log_.is_open()) Log_.close();
};

bool Tintin_reporter::create_dir()
{
    struct stat st;

    if (stat(BASE_PATH, &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
            return true;
        return false;
    }
    if (mkdir(BASE_PATH, 0755) == -1)
        return false;

    return true;
}

bool Tintin_reporter::init()
{
    if (!create_dir())
        return false;
    Info_.open(INFO_PATH, std::ios::out | std::ios::app);
    Error_.open(ERROR_PATH, std::ios::out | std::ios::app);
    Log_.open(LOG_PATH, std::ios::out | std::ios::app);

    if (!Info_ || !Error_ || !Log_)
        return false;
    log(INFO, "Started.");
    return true;
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
    email << "To: " << this->to << "\n";
    email << "Subject: Email From DaemonApp\n\n";
    email << msg;
    email.close();

    std::string cmd = "curl --url 'smtps://smtp.gmail.com:465' "
                      "--ssl-reqd "
                      "--mail-from 'josephardev@gmail.com' "
                      "--mail-rcpt \"" + this->to + "\" "
                      "--user \"josephardev@gmail.com:" + this->pass + "\" "
                      "--upload-file email.txt";
    if (system(cmd.c_str()) != 0)
        this->log(ERROR, std::string("ERROR while sending mail\n").append(cmd));

    unlink("email.txt");
}