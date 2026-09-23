#include <sstream>
#include <stdexcept>

#include "message.h"

namespace mango
{
    std::vector<Byte> Message::Serialize()
    {
        std::vector<Byte> stream;

        stream.push_back(static_cast<Byte>((Type >> 24) & 0xFF));
        stream.push_back(static_cast<Byte>((Type >> 16) & 0xFF));
        stream.push_back(static_cast<Byte>((Type >> 8) & 0xFF));
        stream.push_back(static_cast<Byte>(Type & 0xFF));

        stream.insert(stream.end(), body_.begin(), body_.end());

        return stream;
    }

    void Message::Deserialize(const std::vector<Byte> &data)
    {
        if (data.size() < sizeof(Type))
        {
            std::stringstream errinfo;
            errinfo << "Deserialize fail, too short:" << data.size() << " bytes";
            throw std::runtime_error(errinfo.str());
        }

        Type = (((data[0] & 0xFF) << 24) |
              ((data[1] & 0xFF) << 16) |
              ((data[2] & 0xFF) << 8) |
              ((data[3] & 0xFF)));

        body_ = std::vector<Byte>(data.begin() + 4, data.end());
    }
}