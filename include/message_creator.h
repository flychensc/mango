#pragma once

#include <unordered_map>
#include <functional>
#include <memory>
#include <stdexcept>

#include "message.h"

namespace mango
{
    class MessageCreator
    {
    public:
        using CreatorFunction = std::function<std::shared_ptr<Message>()>;

        static void registerMessageType(int type, CreatorFunction creator);

        static std::shared_ptr<Message> Deserialize(const std::vector<loquat::Byte> &data);

    private:
        static std::unordered_map<int, CreatorFunction> creators_;
    };
}
