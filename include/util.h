#pragma once

#include <regex>
#include <string>

namespace mango
{
    enum class AddressType
    {
        IPv4,
        IPv6,
        UnixSocket,
        Invalid
    };

    AddressType identifyAddressType(const std::string &url)
    {
        // IPv4
        std::regex ipv4Pattern(R"((\d{1,3}\.){3}\d{1,3})");
        // IPv6
        std::regex ipv6Pattern(R"(([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}|::|(([0-9a-fA-F]{1,4}:){1,6}:)|(([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}))");
        // Unix socket
        std::regex unixSocketPattern(R"(^/[^/].*)");

        if (std::regex_match(url, ipv4Pattern))
        {
            return AddressType::IPv4;
        }
        else if (std::regex_match(url, ipv6Pattern))
        {
            return AddressType::IPv6;
        }
        else if (std::regex_match(url, unixSocketPattern))
        {
            return AddressType::UnixSocket;
        }
        else
        {
            return AddressType::Invalid;
        }
    }
}