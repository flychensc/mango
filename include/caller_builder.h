#pragma once

#include "builder.h"
#include "caller.h"

namespace mango
{
    class CallerBuilder : public Builder
    {
    public:
        CallerBuilder(const std::string &unix_path) : Builder(unix_path)
        {
            pollable_ = std::make_shared<Caller>(unix_path);
        }
        CallerBuilder(const std::string &address, int port) : Builder(address, port)
        {
            pollable_ = std::make_shared<Caller>(address, port);
        }

        void joinToEpoll() override
        {
            auto p_sock = std::dynamic_pointer_cast<Caller>(pollable_);
            if (p_sock)
            {
                loquat::Epoll::GetInstance().Join(p_sock->Sock(), p_sock);
            }
        }

        std::shared_ptr<loquat::Pollable> getResult() override
        {
            return pollable_;
        }

    private:
        std::shared_ptr<loquat::Pollable> pollable_;
    };
}
