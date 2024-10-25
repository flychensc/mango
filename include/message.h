#pragma once

#include "context.h"

namespace mango
{
    class Message
    {
    public:
        u_int32_t Id;

        std::vector<Byte> Serialize();

        void Deserialize(const std::vector<Byte> &data);

        virtual void OnCall(Context &context) {}

    protected:
        void setBody(const std::vector<Byte> body) { body_ = body; }

    private:
        std::vector<Byte> body_;
    };
}
