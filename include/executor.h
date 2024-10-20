#pragma once

#include <future>

#include "loquat/include/listener.h"

namespace mango
{
    class Executor : public loquat::Listener
    {
    public:
        void start();
        void stop();

    private:
        std::future<void> fut_;
    };
}
