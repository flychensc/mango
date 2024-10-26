#pragma once

#include <iostream>
#include <string>

#include "message.h"

namespace mango
{
    enum DemoMessage
    {
        PING = 1,
        PONG = 2,
    };

    class PongMessage : public Message
    {
    public:
        PongMessage()
        {
            Type = PONG;
            setBody("PONG");
        }

        void OnCall(Context &context) override
        {
            std::cout << "PONG" << std::endl;
        }

    private:
        void setBody(const std::string &str)
        {
            Message::setBody(std::vector<loquat::Byte>(str.begin(), str.end()));
        }
    };

    class PingMessage : public Message
    {
    public:
        PingMessage()
        {
            Type = PING;
            setBody("PING");
        }

        void OnCall(Context &context) override
        {
            std::cout << "PING" << std::endl;

            // set reply
            context.reply = PongMessage().Serialize();
        }

    private:
        void setBody(const std::string &str)
        {
            Message::setBody(std::vector<loquat::Byte>(str.begin(), str.end()));
        }
    };

    std::shared_ptr<Message> createPingMessage()
    {
        return std::make_shared<PingMessage>();
    }

    std::shared_ptr<Message> createPongMessage()
    {
        return std::make_shared<PongMessage>();
    }
}