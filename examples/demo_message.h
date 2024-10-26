#pragma once

#include <iostream>
#include <string>

#include "message.h"

namespace mango
{
    class PongMessage : public Message
    {
    public:
        PongMessage()
        {
            Id = 2;
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
            Id = 1;
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
}