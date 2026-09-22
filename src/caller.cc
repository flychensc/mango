#include "caller.h"
#include "message_creator.h"
#include "util.h"
#include <spdlog/spdlog.h>

namespace mango
{
    Caller::Caller(const std::string &unix_path) : loquat::Connector(Stream::Type::Framed, determineDomain(unix_path)), recv_state_(RecvState::RECV_MAGIC)
    {
        SetBytesNeeded(4);
        Bind(unix_path);
    }

    Caller::Caller(const std::string &address, int port) : loquat::Connector(Stream::Type::Framed, determineDomain(address)), recv_state_(RecvState::RECV_MAGIC)
    {
        SetBytesNeeded(4);
        Bind(address, port);
    }

    void Caller::OnRecv(std::vector<Byte> data)
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
                spdlog::error("Invalid protocol magic: {:08x}, expected {:08x}", magic, kProtocolMagic);
                Close();
                return;
            }
            SetBytesNeeded(1);
            recv_state_ = RecvState::RECV_VERSION;
            break;
        }
        case RecvState::RECV_VERSION: {
            if (data[0] != kProtocolVersion)
            {
                spdlog::warn("Unsupported protocol version: {}, expected {}", data[0], kProtocolVersion);
            }
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
            spdlog::debug("session id: {}", last_recv_sess_id_);
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
                spdlog::error("Message too long: {} (max {})", length, kMaxMessageLen);
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
            auto session = session_manager_.getSession(last_recv_sess_id_);
            if (session)
            {
                session->getContext().reply = data;
                session->notify();
            }
            else
            {
                spdlog::debug("No session found for id {}, discarding reply (likely timed out)", last_recv_sess_id_);
            }
            SetBytesNeeded(4);
            recv_state_ = RecvState::RECV_MAGIC;
            break;
        }
        }
    }

    void Caller::OnClose(int sock_fd)
    {
        spdlog::debug("Caller connection {} closed, notifying {} pending sessions", sock_fd, session_manager_.count());
        session_manager_.apply([](std::shared_ptr<Session> session)
                               { session->notifyWithError(ErrorCode::CONNECTION_CLOSED, "Connection closed before reply received"); });
    }

    void Caller::cast(Message &message)
    {
        spdlog::debug("Caller cast executor");

        auto session = session_manager_.createSession();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            // Serialize Message
            auto data = message.Serialize();
            // Enqueue Header
            Enqueue(packHeader(session->getId(), data.size()));
            // Enqueue Message
            Enqueue(data);
        }

        // remove session
        session_manager_.removeSession(session->getId());
    }

    std::shared_ptr<Message> Caller::call(Message &message, std::chrono::milliseconds timeout)
    {
        spdlog::debug("Caller call executor");

        auto session = session_manager_.createSession();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto data = message.Serialize();
            Enqueue(packHeader(session->getId(), data.size()));
            Enqueue(data);
        }

        // wait reply with timeout
        bool ok = session->wait_for(timeout);

        auto &ctx = session->getContext();
        if (!ok)
        {
            spdlog::error("RPC call timed out for session {}", session->getId());
            session_manager_.removeSession(session->getId());
            return nullptr;
        }
        if (ctx.error_code != ErrorCode::OK)
        {
            spdlog::error("RPC call failed for session {}: {}", session->getId(), ctx.error_message);
            session_manager_.removeSession(session->getId());
            return nullptr;
        }

        auto reply = MessageCreator::Deserialize(ctx.reply);
        if (!reply)
        {
            spdlog::error("Failed to deserialize reply for session {}", session->getId());
        }
        else
        {
            spdlog::debug("reply type {}", reply->getType());
        }

        session_manager_.removeSession(session->getId());
        return reply;
    }
}