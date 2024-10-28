#include "async_executor.h"
#include "message_creator.h"
#include <spdlog/spdlog.h>
#include <sys/timerfd.h>
#include "loquat/include/epoll.h"

namespace mango
{
    CycleTimer::CycleTimer(int period_in_msec)
    {
        timer_fd_ = timerfd_create(CLOCK_MONOTONIC, 0);
        if (timer_fd_ == -1)
        {
            throw std::runtime_error("AsyncExecutor:timerfd_create");
        }

        struct itimerspec new_value;
        new_value.it_value.tv_sec = 1;
        new_value.it_value.tv_nsec = 0;
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_nsec = 1000000 * period_in_msec;

        timerfd_settime(timer_fd_, 0, &new_value, nullptr);
    }

    CycleTimer::~CycleTimer()
    {
        close(timer_fd_);
        spdlog::debug("~CycleTimer {}", timer_fd_);
    }

    void CycleTimer::OnRead(int sock_fd)
    {
        if (timeoutCallback_)
        {
            timeoutCallback_();
        }
    }

    AsyncExecutor::AsyncExecutor(int listen_fd) : loquat::Connection(Stream::Type::Framed, listen_fd), recv_state_(RecvState::RECV_ID_LENGTH)
    {
        SetBytesNeeded(1);
        cycle_timer_ = std::make_shared<CycleTimer>(50);
        cycle_timer_->registerTimeoutCallback([this]()
                                              { pollRpcCallStatus(); });
        loquat::Epoll::GetInstance().Join(cycle_timer_->TimerFd(), cycle_timer_);
    }

    AsyncExecutor::~AsyncExecutor()
    {
        loquat::Epoll::GetInstance().Leave(cycle_timer_->TimerFd());
    }

    void AsyncExecutor::OnRecv(const std::vector<loquat::Byte> &data)
    {
        switch (recv_state_)
        {
        case RecvState::RECV_ID_LENGTH:
            spdlog::debug("RECV_ID_LENGTH");
            // action
            {
                SetBytesNeeded(data[0]);
            }
            // next state
            recv_state_ = RecvState::RECV_ID_VALUE;
            break;

        case RecvState::RECV_ID_VALUE:
            spdlog::debug("RECV_ID_VALUE");
            // action
            {
                last_recv_sess_id_.assign(data.begin(), data.end());
                SetBytesNeeded(2);

                spdlog::debug("last_recv_sess_id_ {}", last_recv_sess_id_);
            }
            // next state
            recv_state_ = RecvState::RECV_MSG_LENGTH;
            break;

        case RecvState::RECV_MSG_LENGTH:
            spdlog::debug("RECV_MSG_LENGTH");
            // action
            {
                u_int16_t length = (((data[0] & 0xFFU) << 8) | (data[1] & 0xFFU));
                SetBytesNeeded(length);

                spdlog::debug("Message length {}", length);
            }
            // next state
            recv_state_ = RecvState::RECV_MSG_VALUE;
            break;

        case RecvState::RECV_MSG_VALUE:
            spdlog::debug("RECV_MSG_VALUE");
            // action
            {
                // Deserialize Message
                auto message = MessageCreator::Deserialize(data);
                spdlog::debug("message type {}", message->Type);

                // Execute
                auto asyncCtx = std::make_unique<AsyncContext>();
                asyncCtx->session_id = last_recv_sess_id_;
                asyncCtx->future = std::async(std::launch::async, [this, message]
                                              {
                                                Context context;
                                                message->OnCall(context);
                                                context.is_completed = true;
                                                return context.reply; });
                asyncContexts_.push_back(std::move(asyncCtx));

                SetBytesNeeded(1);
            }
            // next state
            recv_state_ = RecvState::RECV_ID_LENGTH;
            break;
        }
    }

    void AsyncExecutor::pollRpcCallStatus()
    {
        asyncContexts_.remove_if([this](auto &item)
                                 {
            if (item->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                auto reply = item->future.get();
                if (reply.size() > 0)
                {
                    // Enqueue Header
                    Enqueue(packHeader(item->session_id, reply.size()));
                    // Enqueue Reply
                    Enqueue(reply);

                    spdlog::debug("async reply {} bytes", reply.size());
                }
                else
                {
                    spdlog::debug("nothing to async reply");
                }
                return true;
            }
            return false; });
    }

    void AsyncExecutor::OnClose(int sock_fd)
    {
        spdlog::debug("Executor {} closed", sock_fd);

        if (close_callback_)
        {
            close_callback_(sock_fd);
        }
    }

    void AsyncExecutor::registerCloseHandler(std::function<void(int)> callback)
    {
        close_callback_ = callback;
    }

    AsyncExecutorService::AsyncExecutorService(const std::string &unix_path) : loquat::Listener(determineDomain(unix_path), kMaxConnections)
    {
        Listen(unix_path);
    }

    AsyncExecutorService::AsyncExecutorService(const std::string &address, int port) : loquat::Listener(determineDomain(address), kMaxConnections)
    {
        Listen(address, port);
    }

    void AsyncExecutorService::OnAccept(int listen_sock)
    {
        auto executor_ptr = std::make_shared<AsyncExecutor>(listen_sock);

        spdlog::debug("Accept a executor {}", executor_ptr->Sock());

        executor_ptr->registerCloseHandler([this](int sock_fd)
                                           { removeExecutor(sock_fd); });

        loquat::Epoll::GetInstance().Join(executor_ptr->Sock(), executor_ptr);
        insertExecutor(executor_ptr->Sock(), executor_ptr);
    }

    void AsyncExecutorService::insertExecutor(int sock_fd, std::shared_ptr<AsyncExecutor> ptr)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        executors_.insert({sock_fd, ptr});
    }

    void AsyncExecutorService::removeExecutor(int sock_fd)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        executors_.erase(sock_fd);
    }
}
