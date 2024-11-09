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

    TEST(SessionManager, creatd)
    {
        mango::SessionManager sessMgr;

        auto sess_1 = sessMgr.createSession();
        auto sess_2 = sessMgr.createSession();

        EXPECT_NE(sessMgr.getSession(sess_1->getId()), nullptr);
        EXPECT_NE(sessMgr.getSession(sess_2->getId()), nullptr);

        sessMgr.removeSession(sess_1->getId());
        EXPECT_EQ(sessMgr.getSession(sess_1->getId()), nullptr);
        EXPECT_NE(sessMgr.getSession(sess_2->getId()), nullptr);

        sessMgr.removeSession(sess_2->getId());
        EXPECT_EQ(sessMgr.getSession(sess_1->getId()), nullptr);
        EXPECT_EQ(sessMgr.getSession(sess_2->getId()), nullptr);
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

    class ThreadPool
    {
    public:
        ThreadPool(size_t threadCount) : stopFlag(false)
        {
            // 初始化线程并进入 wait 状态
            for (size_t i = 0; i < threadCount; ++i)
            {
                threads_.emplace_back([this, i]
                                      {
                std::unique_lock<std::mutex> lock(mutex_);
                while (!stopFlag) {
                    // 等待任务或停止信号
                    condition_.wait(lock, [this] { return stopFlag || task_; });
                    if (stopFlag) return;  // 如果 stopFlag 为真，线程退出

                    // 执行任务
                    if (task_) task_(i);

                    // 任务完成后清空任务指针
                    task_ = nullptr;
                } });
            }
        }

        ~ThreadPool()
        {
            // 设置停止标志并通知所有线程
            {
                std::lock_guard<std::mutex> lock(mutex_);
                stopFlag = true;
            }
            condition_.notify_all();
            for (std::thread &thread : threads_)
            {
                if (thread.joinable())
                {
                    thread.join();
                }
            }
        }

        void apply(const std::function<void(size_t)> &task)
        {
            // 设定任务并通知所有线程
            {
                std::lock_guard<std::mutex> lock(mutex_);
                task_ = task;
            }
            condition_.notify_all();
        }

    private:
        std::vector<std::thread> threads_;
        std::mutex mutex_;
        std::condition_variable condition_;
        std::function<void(size_t)> task_; // 任务，传入线程的索引
        std::atomic<bool> stopFlag;        // 线程池停止标志
    };

    TEST(Session, apply)
    {
        ThreadPool pool(10);
        mango::SessionManager sessMgr;

        pool.apply([&sessMgr](size_t id)
                   {
            auto sess = sessMgr.createSession();
            sess->wait();
            EXPECT_TRUE(sess->getContext().is_completed); });

        std::this_thread::sleep_for(std::chrono::seconds(1));

        sessMgr.apply([](std::shared_ptr<mango::Session> session)
                      { session->notify(); });
    }
}
