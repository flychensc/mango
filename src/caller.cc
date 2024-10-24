#include "caller.h"
#include "message.h"
#include "util.h"
#include <spdlog/spdlog.h>
#include "loquat/include/epoll.h"

namespace mango
{
    Caller::Caller(const std::string &unix_path) : loquat::Connector(determineDomain(unix_path))
    {
        Bind(unix_path);
    }

    Caller::Caller(const std::string &address, int port) : loquat::Connector(determineDomain(address))
    {
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

    void Caller::OnRecv(std::vector<Byte> &data)
    {
        spdlog::debug("Caller recv data");

        // recv header(session id)
        // recv body
        
        Message message;
        message.Deserialize(data);
        // todo: locate session
        // notify
    }

    void Caller::cast(Message &message)
    {
        spdlog::debug("Caller cast executor");

        // send message
        Enqueue(message.Serialize());
    }

    std::unique_ptr<Message> Caller::call(Message &message)
    {
        spdlog::debug("Caller call executor");

        auto session = session_manager_.createSession();

        // todo: bind session id

        // send message
        Enqueue(message.Serialize());
        // wait reply
        session->wait();

        // return reply
        auto reply = std::make_unique<Message>();
        reply->Deserialize(session->getContext().reply);
        return reply;
    }
}
