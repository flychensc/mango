#pragma once

#include "builder.h"
#include "async_executor.h"
#include "loquat/include/epoll.h"

namespace mango
{
    class AsyncExecutorServiceBuilder : public Builder
    {
    public:
        AsyncExecutorServiceBuilder(const std::string &unix_path) : Builder(unix_path)
        {
            pollable_ = std::make_shared<AsyncExecutorService>(unix_path);
        }
        AsyncExecutorServiceBuilder(const std::string &address, int port) : Builder(address, port)
        {
            pollable_ = std::make_shared<AsyncExecutorService>(address, port);
        }

        void joinToEpoll() override
        {
            auto p_sock = std::dynamic_pointer_cast<AsyncExecutorService>(pollable_);
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
