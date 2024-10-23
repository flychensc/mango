#include "session_manager.h"

#include <future>
#include <thread>

#include "gtest/gtest.h"
namespace
{
    std::vector<unsigned char> stringToVector(const std::string &str)
    {
        return std::vector<unsigned char>(str.begin(), str.end());
    }

    TEST(SessionManager, uniqueID)
    {
        mango::SessionManager sessMgr;

        for (int i = 0; i < 1000; i++)
        {
            auto sess_1 = sessMgr.createSession();
            auto sess_2 = sessMgr.createSession();
            EXPECT_NE(sess_1->getId(), sess_2->getId());
        }
    }

    TEST(Session, confirmReference)
    {
        mango::SessionManager sessMgr;

        auto sess = sessMgr.createSession();
        auto &ctx = sess->getContext();

        EXPECT_EQ(sess->getContext().is_completed, false);
        EXPECT_EQ(sess->getContext().reply.size(), 0);

        ctx.is_completed = true;
        EXPECT_EQ(sess->getContext().is_completed, true);

        ctx.reply = stringToVector("Modified");
        EXPECT_EQ(sess->getContext().reply, stringToVector("Modified"));
    }

    TEST(Session, waitAndNotify)
    {
        mango::SessionManager sessMgr;

        auto sess = sessMgr.createSession();

        auto future = std::async(std::launch::async, [sess]
                                 {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                    sess->notify(); });
        sess->wait();
        EXPECT_EQ(future.wait_for(std::chrono::milliseconds(500)), std::future_status::ready);
    }
}
