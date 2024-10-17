#pragma once

#include "loquat/include/io_buffer.h"

namespace mango
{
    using Byte = loquat::Byte;

    struct Context
    {
        bool is_completed;
        std::vector<Byte> reply;
    };
}
