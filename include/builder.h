#pragma once

#include <memory>
#include "loquat/include/pollable.h"

namespace mango
{
    class Builder
    {
    public:
        Builder([[maybe_unused]] const std::string &unix_path) {}
        Builder([[maybe_unused]] const std::string &address, [[maybe_unused]] int port) {}
        virtual void joinToEpoll() = 0;
        virtual std::shared_ptr<loquat::Pollable> getResult() = 0;
    };
}