#pragma once

#include <cstdint>

namespace mango
{
    // Protocol magic number: "MG\x00\x01"
    constexpr uint32_t kProtocolMagic   = 0x4D470001u;
    constexpr uint8_t  kProtocolVersion = 1;
    constexpr uint8_t  kMaxSessionIdLen = 255;
    constexpr uint32_t kMaxMessageLen   = 10u * 1024u * 1024u; // 10 MB

    enum class RecvState
    {
        RECV_MAGIC,       // 4 bytes
        RECV_VERSION,     // 1 byte
        RECV_ID_LENGTH,   // 1 byte
        RECV_ID_VALUE,
        RECV_MSG_LENGTH,  // 4 bytes (big-endian, uint32_t)
        RECV_MSG_VALUE
    };
}