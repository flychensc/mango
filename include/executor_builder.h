#pragma once

#include "builder.h"
#include "executor.h"

namespace mango
{
    class ExecutorBuilder : public Builder
    {
    public:
        ExecutorBuilder(const std::string &unix_path) : Builder(unix_path)
        {
            pollable_ = std::make_shared<Executor>(unix_path);
        }
        ExecutorBuilder(const std::string &address, int port) : Builder(address, port)
        {
            pollable_ = std::make_shared<Executor>(address, port);
        }

        void joinToEpoll() override
        {
            auto p_sock = std::dynamic_pointer_cast<Executor>(pollable_);
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
