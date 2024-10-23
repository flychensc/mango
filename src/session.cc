#include "session.h"

namespace mango
{
    void Session::wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]
                 { return context_.is_completed; });
    }

    void Session::notify()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            context_.is_completed = true;
        }
        cv_.notify_all();
    }
}