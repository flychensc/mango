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

        Message message;
        message.Deserialize(data);
        // todo: message.OnCall();
    }

    void Caller::call(Message &message)
    {
        spdlog::debug("Caller call executor");

        // send message
        Enqueue(message.Serialize());
        // todo: wait reply
    }
}
