#include "../hpp/Daemon.hpp"
#include <iostream>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>


DaemonApp* DaemonApp::instance_ = nullptr;

DaemonApp::DaemonApp(Tintin_reporter *report , DaemonServer * daemon_server)
: lock_path_("/var/lock/matt_daemon.lock"), lock_fd_(-1) {
    report_  = report;
    stop_ = 0;
    this->daemon_server = daemon_server;
};

DaemonApp::DaemonApp()
{
    lock_path_="/var/lock/matt_daemon.lock";
    lock_fd_ = -1;
    report_  = nullptr;
    stop_ = 0;
    this->daemon_server = daemon_server;
};

DaemonApp::~DaemonApp()
{
    remove_lock();
};

void DaemonApp::remove_lock()
{
    if (lock_fd_ != -1)
    {
        flock(lock_fd_, LOCK_UN);
        close(lock_fd_);
        lock_fd_ = -1;
        unlink(lock_path_.c_str());
    }
    report_->log(INFO, "Quitting.");
}

bool DaemonApp::create_lock()
{
    lock_fd_ = open(lock_path_.c_str(), O_RDWR | O_CREAT | O_EXCL, 0644);
    if(lock_fd_ < 0)
    {
        std::cerr << "Can't open :" << lock_path_ << std::endl;
        return false;
    }
    if (flock(lock_fd_, LOCK_EX | LOCK_NB) < 0)
        return false;
    ftruncate(lock_fd_, 0);
    return true;
}

bool DaemonApp::daemonize()
{
    report_->log(INFO, "Entering Daemon mode.");
    std::cout << "parent fork" << std::endl;
    std::cout << "sesion id " << getsid(0) << "  proc id " << getpid() << "  group id " << getpgid(0) << std::endl;
    pid_t pid = fork();
    if (pid < 0)
        return false;
    if (pid == 0)
    {
        std::cout << "child" << std::endl;
        std::cout << "sesion id " << getsid(0) << "  proc id " << getpid() << "  group id " << getpgid(0) << std::endl;
    }
    if (pid > 0)
        exit(0);
    if (setsid() < 0)
        return false;
    std::cout << "after session created" << std::endl;
    std::cout << "sesion id " << getsid(0) << "  proc id " << getpid() << "  group id " << getpgid(0) << std::endl;
    pid = fork();
    if (pid == 0)
    {
        std::cout << "grandchild" << std::endl;
        std::cout << "sesion id " << getsid(0) << "  proc id " << getpid() << "  group id " << getpgid(0) << std::endl;
    }
    std::cout << "after second fork" << std::endl;
    if (pid < 0)
        return false;
    if (pid > 0)
        exit(0);
    
    umask(0);
    if (chdir("/") != 0)
        return false;
    close(STDERR_FILENO);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    char buf[64];
    int n = snprintf(buf, sizeof(buf), "%d\n", getpid());
    write(lock_fd_, buf, n);
    report_->log(INFO, std::string("started. PID: ").append(std::to_string(getpid())));
    return true;
}
void DaemonApp::signal_handler(int sig)
{
    if (instance_)
        instance_->stop_ = sig;
}
bool DaemonApp::setup_signals()
{
    instance_ = this;
    signal(SIGINT,&DaemonApp::signal_handler);
    signal(SIGTERM, &DaemonApp::signal_handler);
    signal(SIGQUIT, &DaemonApp::signal_handler);
    return true;
}

bool DaemonApp::init(){
    if (geteuid() != 0)
    {
        std::cerr << "Mat_daemon must run as root." << std::endl;
        return false;
    }
    if (!report_->init())
    {
        std::cerr << "Failed to open log file." << std::endl;
        return false;
    }
    if (!create_lock())
    {
        report_->log(ERROR, " Error file locked. " + lock_path_);
        return false;
    }
    if (this->daemon_server->socketBindListen())
        return false;

    if (!daemonize())
    {
        report_->log(ERROR,std::string("Failed to daemonize: ").append(std::to_string(getpid())));
        return false;
    }
    if (!setup_signals())
    {
        report_->log(ERROR, "Faild to setup signals");
        return false;
    }
    return true;
}
std::string DaemonApp::signal_name(int sig)
{
    switch (sig)
    {
        case SIGINT:
            return "SIGINT";
        case SIGTERM:
            return "SIGTERM";
        case SIGQUIT:
            return "SIGQUIT";
        default:
            return std::to_string(sig);
    }
};

int DaemonApp::run()
{
    this->daemon_server->run();
    if (instance_->stop_)
    {
        std::string name = signal_name(instance_->stop_);
        report_->log(INFO, std::string("Signal handler. type: ").append(name));
        //report_->send_mail(std::string("Daemon Quitted Using Signal Of Type ").append(name));
    }

    report_->log(LOG, "User wa3333: ");
    return 0;
};
