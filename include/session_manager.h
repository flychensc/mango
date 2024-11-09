#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "session.h"

namespace mango
{
    class SessionManager
    {
    public:
        std::shared_ptr<Session> createSession();
        std::shared_ptr<Session> getSession(const std::string &id);
        void removeSession(const std::string &id);
        void apply(const std::function<void(std::shared_ptr<Session>)> &func);

    private:
        static std::string generateSessionId();

        std::mutex mutex_;
        std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;
    };
}
