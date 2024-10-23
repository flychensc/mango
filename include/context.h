#pragma once

#include "loquat/include/io_buffer.h"

namespace mango
{
    using Byte = loquat::Byte;

    struct Context
    {
        Context() : is_completed(false) {}

        bool is_completed;
        std::vector<Byte> reply;
    };
}
