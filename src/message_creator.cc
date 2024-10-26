#include "message_creator.h"

namespace mango
{
    std::unordered_map<int, MessageCreator::CreatorFunction> MessageCreator::creators_;

    void MessageCreator::registerMessageType(int type, CreatorFunction creator)
    {
        if (creators_.find(type) != creators_.end())
        {
            throw std::runtime_error("Duplicanted message type");
        }

        creators_[type] = creator;
    }

    std::shared_ptr<Message> MessageCreator::Deserialize(const std::vector<loquat::Byte> &data)
    {
        if (data.size() < 4)
        {
            return nullptr;
        }

        int type = (((data[0] & 0xFF) << 24) |
                    ((data[1] & 0xFF) << 16) |
                    ((data[2] & 0xFF) << 8) |
                    ((data[3] & 0xFF)));

        if (creators_.find(type) != creators_.end())
        {
            auto message = creators_[type]();
            message->Deserialize(data);
            return message;
        }
        return nullptr;
    }
}
