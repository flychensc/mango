#include "executor.h"
#include "message.h"
#include <spdlog/spdlog.h>
#include "loquat/include/epoll.h"

namespace mango
{
    void Executor::OnRecv(std::vector<loquat::Byte> &data)
    {
        spdlog::debug("Executor recv data");

        Message message;
        message.Deserialize(data);
        // todo: message.OnCall();
    }

    void Executor::OnClose(int sock_fd)
    {
        spdlog::debug("Executor {} closed", sock_fd);

        if (close_callback_)
        {
            close_callback_(sock_fd);
        }
    }

    void Executor::registerCloseHandler(std::function<void(int)> callback)
    {
        close_callback_ = callback;
    }

    ExecutorService::ExecutorService(const std::string &unix_path) : loquat::Listener(determineDomain(unix_path))
    {
        Listen(unix_path);
    }

    ExecutorService::ExecutorService(const std::string &address, int port) : loquat::Listener(determineDomain(address))
    {
        Listen(address, port);
    }

    void ExecutorService::OnAccept(int listen_sock)
    {
        auto executor_ptr = std::make_shared<Executor>(listen_sock);

        spdlog::debug("Accept a executor {}", executor_ptr->Sock());

        executor_ptr->registerCloseHandler([this](int sock_fd)
                                           {
                                            loquat::Epoll::GetInstance().Leave(sock_fd);
                                            removeExecutor(sock_fd); });

        loquat::Epoll::GetInstance().Join(executor_ptr->Sock(), executor_ptr);
        insertExecutor(executor_ptr->Sock(), executor_ptr);
    }

    void ExecutorService::insertExecutor(int sock_fd, std::shared_ptr<Executor> ptr)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        executors_.insert({sock_fd, ptr});
    }

    void ExecutorService::removeExecutor(int sock_fd)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        executors_.erase(sock_fd);
    }

    void ExecutorService::start()
    {
        fut_ = std::async(std::launch::async, []
                          { loquat::Epoll::GetInstance().Wait(); });
    }

    void ExecutorService::stop()
    {
        loquat::Epoll::GetInstance().Terminate();
        fut_.wait();
    }
}
