#ifndef DAEMON_HPP
#define DAEMON_HPP

#include <string>
#include <signal.h>
#include <sys/stat.h>

#include "Tintin_reporter.hpp"
#include "DaemonServer.hpp"
class DaemonApp
{
    public:
        DaemonApp(Tintin_reporter * report, DaemonServer * daemon_server);
        DaemonApp();
        ~DaemonApp();

        bool init();
        int run();
    private:
        std::string lock_path_;
        int lock_fd_;
        static DaemonApp* instance_;
        static void signal_handler(int sig);
        volatile sig_atomic_t stop_;
        DaemonApp(const DaemonApp& other) = delete;
        DaemonApp& operator=(const DaemonApp& other) = delete;
        bool create_lock();
        bool daemonize();
        bool setup_signals();
        void remove_lock();
        std::string signal_name(int sig);
        Tintin_reporter *report_;
        DaemonServer * daemon_server;
};

#endif
