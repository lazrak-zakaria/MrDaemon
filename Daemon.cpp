#include "Daemon.hpp"
#include <iostream>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include "DaemonServer.hpp"

DaemonApp* DaemonApp::instance_ = nullptr;

DaemonApp::DaemonApp(Tintin_reporter *report)
: lock_path_("/var/lock/matt_daemon.lock"), lock_fd_(-1) {
    report_  = report;
    stop_ = 0;

};

DaemonApp::~DaemonApp() {}

bool DaemonApp::create_lock()
{
    lock_fd_ = open(lock_path_.c_str(), O_RDWR | O_CREAT, 0644);
    if(lock_fd_ < 0)
    {
        std::cerr << "Can't open: " << lock_path_ << std::endl;
        return false;
    }
    if (flock(lock_fd_, LOCK_EX | LOCK_NB) < 0)
    {
        std::cerr << "Can't lock: " << lock_path_ << std::endl;
        return false;
    }
    ftruncate(lock_fd_, 0);
    return true;
}

bool DaemonApp::daemonize()
{
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
    
    signal(SIGHUP, SIG_IGN);
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
    // int fd = open("/dev/null", O_RDWR);
    // if (fd >= 0)
    // {
    //     dup2(fd, STDIN_FILENO);
    //     dup2(fd, STDOUT_FILENO);
    //     dup2(fd, STDERR_FILENO);
    //     if (fd > 2)
    //         close(fd);
    // }
    char buf[64];
    int n = snprintf(buf, sizeof(buf), "%d\n", (int)getpid());
    write(lock_fd_, buf, n);
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
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &DaemonApp::signal_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGQUIT, &sa, nullptr);
    return true;
}

bool DaemonApp::init(){
    if (geteuid() != 0)
    {
        std::cerr << "Mat_daemon must run as root." << std::endl;
        return false;
    }
    if (report_->init())
    {
        std::cerr << "Failed to open log file." << std::endl;
        return false;
    }
    if (!create_lock())
    {
        report_->log(ERROR, "Failed to create lock: " + lock_path_);
        return false;
    }
    if (!daemonize())
    {
        std::string d = ("Failed to daemonize: ");
        d.append(std::to_string(getpid()));
        report_->log(ERROR,d);
        return false;
    }
    report_->log(INFO, "Daemon initialized successfully.");
    if (!setup_signals())
    {
        report_->log(ERROR, "Faild to setup signals");
        return false;
    }
    return true;
}

int DaemonApp::run()
{
    
    DaemonServer daemon_server(report_);


  
    daemon_server.run();
        if (instance_->stop_)
        {

            std::cout<<"DDDDDDDDDDdsfafdsafdssdfds\n";
            report_->log(INFO, "Signal handler.");

        }   


            report_->log(INFO, "Quitting.");


    return 0;
};
