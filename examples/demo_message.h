#pragma once

#include <string>

#include "message.h"

namespace mango
{
    class PingMessage : public Message
    {
    public:
        PingMessage()
        {
            Id = 1;
            setBody("PING");
        }

    private:
        void setBody(const std::string &str)
        {
            Message::setBody(std::vector<loquat::Byte>(str.begin(), str.end()));
        }
    };

    class PongMessage : public Message
    {
    public:
        PongMessage()
        {
            Id = 2;
            setBody("PING");
        }

    private:
        void setBody(const std::string &str)
        {
            Message::setBody(std::vector<loquat::Byte>(str.begin(), str.end()));
        }
    };
}