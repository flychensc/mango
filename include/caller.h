#pragma once

#include <future>

#include "message.h"
#include "loquat/include/connector.h"

namespace mango
{
    class Caller : public loquat::Connector
    {
    public:
        Caller(const std::string unix_path);
        Caller(const std::string address, int port);

        void start();
        void stop();

        void call(Message &message);

    private:
        Caller(int domain, const std::string address, int port);
        std::future<void> fut_;
    };
}
