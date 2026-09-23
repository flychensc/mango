#pragma once

#include <cstdint>
#include "context.h"

namespace mango
{
    class Message
    {
    public:
        Message() : Type(0) {}
        virtual ~Message() = default;

        uint32_t getType() const { return Type; }
        void setType(uint32_t t) { Type = t; }

        /**
         * @brief Serialize the message
         * @return Serialized data
         */
        std::vector<Byte> Serialize();

        /**
         * @brief Deserialize message data
         * @param data Serialized data
         */
        void Deserialize(const std::vector<Byte> &data);

        /**
         * @brief Handle when receiving the message
         * @param context Message context
         */
        virtual void OnCall([[maybe_unused]] Context &context) {}

    protected:
        uint32_t Type;

        void setBody(const std::vector<Byte> &body) { body_ = body; }
        const std::vector<Byte> &getBody() const { return body_; }

    private:
        std::vector<Byte> body_;
    };
}