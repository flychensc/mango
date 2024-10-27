#pragma once

#include "context.h"

namespace mango
{
    class Message
    {
    public:
        u_int32_t Type;

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
        virtual void OnCall(Context &context) {}

    protected:
        /**
         * @brief Set the message body
         * @param body The message body
         */
        void setBody(const std::vector<Byte> body) { body_ = body; }
        /**
         * @brief Get the message body
         * @return The message body
         */
        std::vector<Byte> getBody() { return body_; }

    private:
        std::vector<Byte> body_;
    };
}
