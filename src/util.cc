#include <cerrno>
#include <cstring>
#include <regex>
#include <sys/socket.h>
#include "pack.h"
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

    std::vector<loquat::Byte> packHeader(const std::string &session_id, size_t message_length)
    {
        if (session_id.empty() || session_id.size() > kMaxSessionIdLen)
        {
            throw std::invalid_argument("packHeader: invalid session_id length");
        }
        if (message_length > kMaxMessageLen)
        {
            throw std::invalid_argument("packHeader: message too long");
        }

        std::vector<loquat::Byte> header;
        header.reserve(4 + 1 + 1 + session_id.size() + 4);

        // Magic Number (4 bytes, big-endian)
        header.push_back((kProtocolMagic >> 24) & 0xFFu);
        header.push_back((kProtocolMagic >> 16) & 0xFFu);
        header.push_back((kProtocolMagic >> 8)  & 0xFFu);
        header.push_back( kProtocolMagic        & 0xFFu);

        // Version (1 byte)
        header.push_back(kProtocolVersion);

        // Session ID length + content
        header.push_back(static_cast<loquat::Byte>(session_id.size()));
        header.insert(header.end(), session_id.begin(), session_id.end());

        // Message length (4 bytes, big-endian)
        auto ml = static_cast<uint32_t>(message_length);
        header.push_back((ml >> 24) & 0xFFu);
        header.push_back((ml >> 16) & 0xFFu);
        header.push_back((ml >> 8)  & 0xFFu);
        header.push_back( ml        & 0xFFu);

        return header;
    }
}