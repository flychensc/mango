#pragma once

namespace mango
{
    enum class RecvState
    {
        RECV_ID_LENGTH, // 1 byte
        RECV_ID_VALUE,
        RECV_MSG_LENGTH, // 2 bytes
        RECV_MSG_VALUE
    };
}
