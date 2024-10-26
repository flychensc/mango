#include "executor.h"
#include "message_creator.h"
#include <spdlog/spdlog.h>
#include "loquat/include/epoll.h"

namespace mango
{
    Executor::Executor(int listen_fd) : loquat::Connection(Stream::Type::Framed, listen_fd), recv_state_(RecvState::RECV_ID_LENGTH)
    {
        SetBytesNeeded(1);
    }

    void Executor::OnRecv(const std::vector<loquat::Byte> &data)
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
                Context context;
                message->OnCall(context);

                if (context.reply.size() > 0)
                {
                    // Enqueue Header
                    Enqueue(packHeader(last_recv_sess_id_, context.reply.size()));
                    // Enqueue Reply
                    Enqueue(context.reply);
                    context.is_completed = true;

                    spdlog::debug("reply {} bytes", context.reply.size());
                }
                else
                {
                    spdlog::debug("nothing to reply");
                }
                SetBytesNeeded(1);
            }
            // next state
            recv_state_ = RecvState::RECV_ID_LENGTH;
            break;
        }
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
                                           { removeExecutor(sock_fd); });

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

    void ExecutorService::wait()
    {
        fut_.get();
    }

    void ExecutorService::stop()
    {
        loquat::Epoll::GetInstance().Terminate();
        fut_.wait();
    }
}
