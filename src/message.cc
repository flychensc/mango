#include <sstream>
#include <stdexcept>

#include <arpa/inet.h>

#include "message.h"

namespace mango
{
    using namespace std;

    vector<Byte> Message::Serialize()
    {
        vector<Byte> stream;

        stream.push_back((Id >> 24) & 0xFF);
        stream.push_back((Id >> 16) & 0xFF);
        stream.push_back((Id >> 8) & 0xFF);
        stream.push_back(Id & 0xFF);

        stream.insert(stream.end(), body_.begin(), body_.end());

        return stream;
    }

    void Message::Deserialize(const vector<Byte> &data)
    {
        if (data.size() < sizeof(Id))
        {
            stringstream errinfo;
            errinfo << "Deserialize fail, too short:" << data.size() << "bytes";
            throw runtime_error(errinfo.str());
        }

        Id = (((data[0] & 0xFF) << 24) |
              ((data[1] & 0xFF) << 16) |
              ((data[2] & 0xFF) << 8) |
              ((data[3] & 0xFF)));

        body_ = vector<Byte>(data.begin() + 4, data.end());
    }
}
