#pragma once

#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "pack.h"
#include "util.h"
#include "loquat/include/listener.h"

namespace mango
{
    struct AsyncContext
    {
        std::string session_id;
        std::future<std::vector<loquat::Byte>> future;
    };

    class CycleTimer : public loquat::ReadWritable
    {
    public:
        CycleTimer(int period_in_msec);
        ~CycleTimer();

        int TimerFd() { return timer_fd_; }

        virtual void OnRead(int sock_fd) override final;
        void OnWrite(int sock_fd) override final {}

        void registerTimeoutCallback(std::function<void(void)> callback) { timeoutCallback_ = callback; }

    private:
        int timer_fd_;
        std::function<void(void)> timeoutCallback_;
    };

    class AsyncExecutor : public loquat::Connection
    {
    public:
        AsyncExecutor(int listen_fd);
        ~AsyncExecutor();

        void OnClose(int sock_fd) override final;

        void registerCloseHandler(std::function<void(int)> callback);

    protected:
        void OnRecv(const std::vector<loquat::Byte> &data) override final;

    private:
        void pollRpcCallStatus();

        std::function<void(int)> close_callback_;

        RecvState recv_state_;
        std::string last_recv_sess_id_;

        std::shared_ptr<CycleTimer> cycle_timer_;
        std::list<std::unique_ptr<AsyncContext>> asyncContexts_;
    };

    class AsyncExecutorService : public loquat::Listener
    {
    public:
        static const int kMaxConnections = 10;

        AsyncExecutorService(const std::string &unix_path);
        AsyncExecutorService(const std::string &address, int port);

        void OnAccept(int listen_sock) override final;

        void insertExecutor(int sock_fd, std::shared_ptr<AsyncExecutor> ptr);
        void removeExecutor(int sock_fd);

    private:
        std::mutex mutex_;
        std::unordered_map<int, std::shared_ptr<AsyncExecutor>> executors_;
    };
}
