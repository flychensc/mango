#pragma once

#include <condition_variable>
#include <mutex>

#include "context.h"
#include "message.h"

namespace mango
{
    class Session
    {
    public:
        Session(const std::string &id) : id_(id) {}

        std::string getId() const { return id_; }
        struct Context &getContext() { return context_; }

        void wait();
        void notify();

    private:
        std::string id_;
        struct Context context_;

        std::mutex mutex_;
        std::condition_variable cv_;
    };
}
