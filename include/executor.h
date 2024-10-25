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
    class Executor : public loquat::Connection
    {
    public:
        Executor(int listen_fd) : loquat::Connection(listen_fd), recv_state_(RecvState::RECV_ID_LENGTH) {}

        void OnClose(int sock_fd) override final;

        void registerCloseHandler(std::function<void(int)> callback);

    protected:
        void OnRecv(const std::vector<loquat::Byte> &data) override final;

    private:
        std::function<void(int)> close_callback_;

        RecvState recv_state_;
        std::string last_recv_sess_id_;
    };

    class ExecutorService : public loquat::Listener
    {
    public:
        ExecutorService(const std::string &unix_path);
        ExecutorService(const std::string &address, int port);

        void start();
        void wait();
        void stop();

        void OnAccept(int listen_sock) override final;

        void insertExecutor(int sock_fd, std::shared_ptr<Executor> ptr);
        void removeExecutor(int sock_fd);

    private:
        std::future<void> fut_;

        std::mutex mutex_;
        std::unordered_map<int, std::shared_ptr<Executor>> executors_;
    };
}
