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
    this->daemon_server = nullptr;
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
    pid_t pid = fork();
    if (pid < 0)
        return false;
    if (pid > 0)
        exit(0);
    if (setsid() < 0)
        return false;
    pid = fork();
    if (pid < 0)
        return false;
    if (pid > 0)
        exit(0);
    umask(0);
    if (chdir("/") != 0)
        return false;
    close(STDERR_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
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

    for (int sig = 1; sig < NSIG; ++sig) {
        if (sig == SIGKILL || sig == SIGSTOP || sig == SIGCONT ) {
            continue;
        }
        signal(sig, &DaemonApp::signal_handler);
    }

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
#include <csignal>
#include <string>

std::string DaemonApp::signal_name(int sig)
{
    switch (sig)
    {
        case SIGHUP:   return "SIGHUP";
        case SIGINT:   return "SIGINT";
        case SIGQUIT:  return "SIGQUIT";
        case SIGILL:   return "SIGILL";
        case SIGTRAP:  return "SIGTRAP";
        case SIGABRT:  return "SIGABRT";
        case SIGBUS:   return "SIGBUS";
        case SIGFPE:   return "SIGFPE";
        case SIGUSR1:  return "SIGUSR1";
        case SIGSEGV:  return "SIGSEGV";
        case SIGUSR2:  return "SIGUSR2";
        case SIGPIPE:  return "SIGPIPE";
        case SIGALRM:  return "SIGALRM";
        case SIGTERM:  return "SIGTERM";
        case SIGSTKFLT:return "SIGSTKFLT";
        case SIGCHLD:  return "SIGCHLD";
        case SIGCONT:  return "SIGCONT";
        case SIGTSTP:  return "SIGTSTP";
        case SIGTTIN:  return "SIGTTIN";
        case SIGTTOU:  return "SIGTTOU";
        case SIGURG:   return "SIGURG";
        case SIGXCPU:  return "SIGXCPU";
        case SIGXFSZ:  return "SIGXFSZ";
        case SIGVTALRM:return "SIGVTALRM";
        case SIGPROF:  return "SIGPROF";
        case SIGWINCH: return "SIGWINCH";
        case SIGPOLL:  return "SIGPOLL";
        case SIGPWR:   return "SIGPWR";
        case SIGSYS:   return "SIGSYS";
        default:
            return "UNKNOWN(" + std::to_string(sig) + ")";
    }
}


int DaemonApp::run()
{
    this->daemon_server->run();
    if (instance_->stop_)
    {
        std::string name = signal_name(instance_->stop_);
        report_->log(INFO, std::string("Signal handler. type: ").append(name));
        report_->send_mail(std::string("Daemon Quitted Using Signal Of Type ").append(name));
    }
    return 0;
};
