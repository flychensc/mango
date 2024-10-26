#pragma once

#include <future>
#include <memory>
#include <mutex>

#include "message.h"
#include "pack.h"
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

        void cast(Message &message);
        std::shared_ptr<Message> call(Message &message);

    protected:
        void OnRecv(const std::vector<Byte> &data) override final;

    private:
        SessionManager session_manager_;
        std::future<void> fut_;
        std::mutex mutex_;

        RecvState recv_state_;
        std::string last_recv_sess_id_;
    };
}
