#pragma once

#include <memory>
#include "loquat/include/pollable.h"

namespace mango
{
    class Builder
    {
    public:
        Builder(const std::string &unix_path) {}
        Builder(const std::string &address, int port) {}
        virtual void joinToEpoll() = 0;
        virtual std::shared_ptr<loquat::Pollable> getResult() = 0;
    };
}
