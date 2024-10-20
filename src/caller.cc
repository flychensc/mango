#include "caller.h"
#include "loquat/include/epoll.h"
#include "util.h"

namespace mango
{
    Caller::Caller(const std::string unix_path) : Connector(AF_UNIX)
    {
        Bind(unix_path);
    }

    Caller::Caller(int domain, const std::string address, int port) : Connector(domain)
    {
        Bind(address, port);
    }

    void Caller::start()
    {
        fut_ = std::async(std::launch::async, []
                          { loquat::Epoll::GetInstance().Wait(); });
    }

    void Caller::stop()
    {
        loquat::Epoll::GetInstance().Terminate();
        fut_.wait();
    }

    void Caller::call(Message &message)
    {
        // send message
        // wait reply
    }
}
