#pragma once

#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "util.h"
#include "loquat/include/listener.h"

namespace mango
{
    class Executor : public loquat::Connection
    {
    public:
        Executor(int listen_fd) : loquat::Connection(listen_fd) {}

        void OnRecv(std::vector<loquat::Byte> &data) override final;
        void OnClose(int sock_fd) override final;

        void registerCloseHandler(std::function<void(int)> callback);

    private:
        std::function<void(int)> close_callback_;
    };

    class ExecutorService : public loquat::Listener
    {
    public:
        ExecutorService(const std::string &unix_path);
        ExecutorService(const std::string &address, int port);

        void start();
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
