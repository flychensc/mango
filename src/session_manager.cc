#include "session_manager.h"

#include <chrono>
#include <random>
#include <sstream>

namespace mango
{
    std::shared_ptr<Session> SessionManager::createSession()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return std::make_shared<Session>(generateSessionId());
    }

    std::shared_ptr<Session> SessionManager::getSession(const std::string &id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return sessions_.at(id);
    }

    void SessionManager::removeSession(const std::string &id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sessions_.erase(id);
    }

    std::string SessionManager::generateSessionId()
    {
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<int> dist(1000, 9999);

        std::ostringstream oss;
        oss << now << "-" << dist(mt);
        return oss.str();
    }
}
