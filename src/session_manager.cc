#include "session_manager.h"

#include <atomic>
#include <sstream>

namespace mango
{
    namespace
    {
        std::atomic<uint64_t> g_session_counter{0};
    }

    std::shared_ptr<Session> SessionManager::createSession()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto session = std::make_shared<Session>(generateSessionId());
        if (sessions_.find(session->getId()) != sessions_.end())
        {
            throw std::runtime_error("Duplicate session " + session->getId());
        }
        sessions_[session->getId()] = session;
        return session;
    }

    std::shared_ptr<Session> SessionManager::getSession(const std::string &id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = sessions_.find(id);
        return (it != sessions_.end()) ? it->second : nullptr;
    }

    void SessionManager::removeSession(const std::string &id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sessions_.erase(id);
    }

    void SessionManager::apply(const std::function<void(std::shared_ptr<Session>)> &func)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto &pair : sessions_)
        {
            if (pair.second)
            {
                func(pair.second);
            }
        }
    }

    std::string SessionManager::generateSessionId()
    {
        std::ostringstream oss;
        oss << g_session_counter.fetch_add(1, std::memory_order_relaxed);
        return oss.str();
    }
}