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

        /**
         * @brief Start new thread for RPC
         */
        void start();
        /**
         * @brief Wait for the RPC thread to end
         */
        void wait();
        /**
         * @brief Stop RPC thread
         */
        void stop();

        /**
         * @brief Send messages that don't require a reply
         * @param message The entity of request message
         */
        void cast(Message &message);
        /**
         * @brief Send messages, and wait the reply
         * @param message The entity of request message
         * @return The reply message
         */
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
