#pragma once

#include <string>
#include <vector>

#include "loquat/include/io_buffer.h"

namespace mango
{
    int determineDomain(const std::string &url);

    std::vector<loquat::Byte> packHeader(const std::string session_id, size_t message_length);
}
