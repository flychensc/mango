#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "context.h"

namespace mango
{
    class Session
    {
    public:
        Session(const std::string &id) : id_(id) {}

        std::string getId() const { return id_; }
        struct Context &getContext() { return context_; }

        // Blocking wait until notify() or notifyWithError()
        void wait();
        // Wait with timeout. Returns true if completed in time.
        bool wait_for(std::chrono::milliseconds timeout);
        // Complete normally (reply received)
        void notify();
        // Complete with error (e.g. connection closed / timeout)
        void notifyWithError(ErrorCode code, const std::string &msg);

    private:
        std::string id_;
        struct Context context_;

        std::mutex mutex_;
        std::condition_variable cv_;
    };
}