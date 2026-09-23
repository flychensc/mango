#pragma once

#include <string>
#include "loquat/include/io_buffer.h"

namespace mango
{
    using Byte = loquat::Byte;

    enum class ErrorCode
    {
        OK = 0,
        TIMEOUT = 1,
        CONNECTION_CLOSED = 2,
        MESSAGE_TOO_SHORT = 3,
        UNKNOWN_MESSAGE_TYPE = 4,
        INTERNAL_ERROR = 5
    };

    struct Context
    {
        Context() : is_completed(false), error_code(ErrorCode::OK) {}

        bool is_completed;
        ErrorCode error_code;
        std::string error_message;
        std::vector<Byte> reply;
    };
}