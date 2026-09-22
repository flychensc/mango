#include "session.h"

namespace mango
{
    void Session::wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]
                 { return context_.is_completed; });
    }

    bool Session::wait_for(std::chrono::milliseconds timeout)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cv_.wait_for(lock, timeout, [this]
                          { return context_.is_completed; }))
        {
            // Timed out: mark as completed with timeout error so subsequent
            // callers don't block on this session again.
            context_.is_completed = true;
            context_.error_code = ErrorCode::TIMEOUT;
            context_.error_message = "RPC call timed out";
            return false;
        }
        return true;
    }

    void Session::notify()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            context_.is_completed = true;
        }
        cv_.notify_all();
    }

    void Session::notifyWithError(ErrorCode code, const std::string &msg)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            context_.is_completed = true;
            context_.error_code = code;
            context_.error_message = msg;
        }
        cv_.notify_all();
    }
}