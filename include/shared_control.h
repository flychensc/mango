#pragma once

#include <atomic>
#include <functional>
#include <future>

#include "loquat/include/epoll.h"

namespace mango
{
    class SharedControl
    {
    public:
        static std::atomic<bool> isRunning;

        static void startThread(std::function<void()> func)
        {
            if (!isRunning.exchange(true))
            {
                fut_ = std::async(std::launch::async, []
                                  { loquat::Epoll::GetInstance().Wait(); });
            }
        }

        static void wait()
        {
            if (isRunning.load(std::memory_order_acquire))
            {
                fut_.get();
                isRunning.exchange(false);
            }
        }

    private:
        static std::future<void> fut_;
    };
}
