#include "caller.h"
#include "message_creator.h"
#include "util.h"
#include <spdlog/spdlog.h>
#include "loquat/include/epoll.h"

namespace mango
{
    Caller::Caller(const std::string &unix_path) : loquat::Connector(Stream::Type::Framed, determineDomain(unix_path)), recv_state_(RecvState::RECV_ID_LENGTH)
    {
        SetBytesNeeded(1);
        Bind(unix_path);
    }

    Caller::Caller(const std::string &address, int port) : loquat::Connector(Stream::Type::Framed, determineDomain(address)), recv_state_(RecvState::RECV_ID_LENGTH)
    {
        SetBytesNeeded(1);
        Bind(address, port);
    }

    void Caller::start()
    {
        fut_ = std::async(std::launch::async, []
                          { loquat::Epoll::GetInstance().Wait(); });
    }

    void Caller::wait()
    {
        fut_.get();
    }

    void Caller::stop()
    {
        loquat::Epoll::GetInstance().Terminate();
        fut_.wait();
    }

    void Caller::OnRecv(const std::vector<Byte> &data)
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
                // locate session
                auto session = session_manager_.getSession(last_recv_sess_id_);
                if (session)
                {
                    session->getContext().reply = data;
                    // notify
                    session->notify();
                }
                SetBytesNeeded(1);
            }
            // next state
            recv_state_ = RecvState::RECV_ID_LENGTH;
            break;
        }
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

    std::unique_ptr<Message> Caller::call(Message &message)
    {
        spdlog::debug("Caller call executor");

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
        // wait reply
        session->wait();

        // return reply
        auto reply = MessageCreator::Deserialize(session->getContext().reply);
        spdlog::debug("reply type {}", reply->Type);

        // remove session
        session_manager_.removeSession(session->getId());

        // return reply
        return reply;
    }
}
