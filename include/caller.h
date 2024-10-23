#pragma once

#include <future>

#include "message.h"
#include "session_manager.h"
#include "loquat/include/connector.h"

namespace mango
{
    class Caller : public loquat::Connector
    {
    public:
        Caller(const std::string &unix_path);
        Caller(const std::string &address, int port);

        void start();
        void wait();
        void stop();

        void OnRecv(std::vector<Byte> &data) override final;

        void call(Message &message);

    private:
        SessionManager session_manager_;
        std::future<void> fut_;
    };
}
