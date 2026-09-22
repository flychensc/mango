#include "executor.h"
#include "message_creator.h"
#include <spdlog/spdlog.h>
#include "loquat/include/epoll.h"

namespace mango
{
    Executor::Executor(int listen_fd) : loquat::Connection(Stream::Type::Framed, listen_fd), recv_state_(RecvState::RECV_MAGIC)
    {
        SetBytesNeeded(4);
    }

    void Executor::OnRecv(std::vector<loquat::Byte> data)
    {
        switch (recv_state_)
        {
        case RecvState::RECV_MAGIC: {
            uint32_t magic = (uint32_t(data[0] & 0xFFu) << 24) |
                             (uint32_t(data[1] & 0xFFu) << 16) |
                             (uint32_t(data[2] & 0xFFu) << 8)  |
                              uint32_t(data[3] & 0xFFu);
            if (magic != kProtocolMagic)
            {
                spdlog::error("Invalid protocol magic: {:08x}", magic);
                Close();
                return;
            }
            SetBytesNeeded(1);
            recv_state_ = RecvState::RECV_VERSION;
            break;
        }
        case RecvState::RECV_VERSION: {
            SetBytesNeeded(1);
            recv_state_ = RecvState::RECV_ID_LENGTH;
            break;
        }
        case RecvState::RECV_ID_LENGTH: {
            if (data[0] == 0 || data[0] > kMaxSessionIdLen)
            {
                spdlog::error("Invalid session id length: {}", data[0]);
                Close();
                return;
            }
            SetBytesNeeded(data[0]);
            recv_state_ = RecvState::RECV_ID_VALUE;
            break;
        }
        case RecvState::RECV_ID_VALUE: {
            last_recv_sess_id_.assign(data.begin(), data.end());
            SetBytesNeeded(4);
            recv_state_ = RecvState::RECV_MSG_LENGTH;
            break;
        }
        case RecvState::RECV_MSG_LENGTH: {
            uint32_t length = (uint32_t(data[0] & 0xFFu) << 24) |
                              (uint32_t(data[1] & 0xFFu) << 16) |
                              (uint32_t(data[2] & 0xFFu) << 8)  |
                               uint32_t(data[3] & 0xFFu);
            if (length > kMaxMessageLen)
            {
                spdlog::error("Message too long: {}", length);
                Close();
                return;
            }
            if (length == 0)
            {
                SetBytesNeeded(4);
                recv_state_ = RecvState::RECV_MAGIC;
                break;
            }
            SetBytesNeeded(length);
            recv_state_ = RecvState::RECV_MSG_VALUE;
            break;
        }
        case RecvState::RECV_MSG_VALUE: {
            auto message = MessageCreator::Deserialize(data);
            if (!message)
            {
                spdlog::error("Unknown message type, dropping");
                SetBytesNeeded(4);
                recv_state_ = RecvState::RECV_MAGIC;
                break;
            }
            spdlog::debug("message type {}", message->getType());

            Context context;
            message->OnCall(context);

            if (context.reply.size() > 0)
            {
                Enqueue(packHeader(last_recv_sess_id_, context.reply.size()));
                Enqueue(context.reply);
                spdlog::debug("reply {} bytes", context.reply.size());
            }
            SetBytesNeeded(4);
            recv_state_ = RecvState::RECV_MAGIC;
            break;
        }
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

    ExecutorService::ExecutorService(const std::string &unix_path) : loquat::Listener(determineDomain(unix_path), kMaxConnections)
    {
        Listen(unix_path);
    }

    ExecutorService::ExecutorService(const std::string &address, int port) : loquat::Listener(determineDomain(address), kMaxConnections)
    {
        Listen(address, port);
    }

    void ExecutorService::OnAccept(int listen_sock)
    {
        auto executor_ptr = std::make_shared<Executor>(listen_sock);

        spdlog::debug("Accept a executor {}", executor_ptr->Sock());

        executor_ptr->registerCloseHandler([this](int sock_fd)
                                           { removeExecutor(sock_fd); });

        loquat::Epoll::GetInstance()->Join(executor_ptr->Sock(), executor_ptr);
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
}