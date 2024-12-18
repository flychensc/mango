#include <regex>
#include <sys/socket.h>
#include "util.h"

namespace mango
{
    int determineDomain(const std::string &url)
    {
        // IPv4
        const std::regex ipv4Pattern(R"((\d{1,3}\.){3}\d{1,3})");
        // IPv6
        const std::regex ipv6Pattern(R"(([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}|(([0-9a-fA-F]{0,6}:):[0-9a-fA-F]{0,4})|(([0-9a-fA-F]{1,4}:){1,6}:)|(([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}))");
        // Unix socket
        const std::regex unixSocketPattern(R"(^/[^/].*)");

        if (std::regex_match(url, ipv4Pattern))
        {
            return AF_INET;
        }
        else if (std::regex_match(url, ipv6Pattern))
        {
            return AF_INET6;
        }
        else if (std::regex_match(url, unixSocketPattern))
        {
            return AF_UNIX;
        }
        else
        {
            throw std::invalid_argument("Unsupported address " + url);
        }
    }

    std::vector<loquat::Byte> packHeader(const std::string session_id, size_t message_length)
    {
        std::vector<loquat::Byte> header;

        // Length of Session Id
        header.push_back(session_id.size());

        // Content of Session Id
        header.insert(header.end(), session_id.begin(), session_id.end());

        // Length of Message
        header.push_back((message_length >> 8) & 0xFF);
        header.push_back(message_length & 0xFF);

        return header;
    }
}