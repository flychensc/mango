#include <atomic>
#include <memory>
#include <future>
#include <queue>
#include <thread>

#include "caller_builder.h"
#include "async_executor_builder.h"
#include "director.h"
#include "message_creator.h"

#include "gtest/gtest.h"

namespace
{
    class TestMessage : public mango::Message
    {
    public:
        TestMessage()
        {
            Type = 8515;
        }

        void OnCall(mango::Context &context) override
        {
            EXPECT_EQ(getMessage(), "Happy to see you, Emmy~");

            TestMessage reply;
            reply.setMessage("Good to see you, Emma~");
            context.reply = reply.Serialize();
        }

        void setMessage(const std::string &msg)
        {
            Message::setBody(std::vector<loquat::Byte>(msg.begin(), msg.end()));
        }
        std::string getMessage()
        {
            auto body = Message::getBody();
            return std::string(body.begin(), body.end());
        }
    };

    std::shared_ptr<mango::Message> createTestMessage()
    {
        return std::make_shared<TestMessage>();
    }

    class SlowMessage : public mango::Message
    {
    public:
        SlowMessage()
        {
            Type = 942936070;
        }

        void OnCall(mango::Context &context) override
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            TestMessage reply;
            reply.setMessage("I'm June!");
            context.reply = reply.Serialize();
        }
    };

    std::shared_ptr<mango::Message> createSlowMessage()
    {
        return std::make_shared<SlowMessage>();
    }

    class FastMessage : public mango::Message
    {
    public:
        FastMessage()
        {
            Type = 422050215;
        }

        void OnCall(mango::Context &context) override
        {
            TestMessage reply;
            reply.setMessage("I'm Lili!");
            context.reply = reply.Serialize();
        }
    };

    std::shared_ptr<mango::Message> createFastMessage()
    {
        return std::make_shared<FastMessage>();
    }

    class SharedControl
    {
    private:
        static std::atomic<bool> isRunning;
        static std::future<void> fut;

        static std::shared_ptr<mango::AsyncExecutorService> rpc_service;

        static void createService()
        {
            std::shared_ptr<mango::Builder> builder = std::make_shared<mango::AsyncExecutorServiceBuilder>("127.0.0.1", 2012);
            mango::Director director;
            director.setBuilder(builder);

            director.construct();

            rpc_service = std::dynamic_pointer_cast<mango::AsyncExecutorService>(builder->getResult());
        }

        static void createCaller()
        {
            std::shared_ptr<mango::Builder> builder = std::make_shared<mango::CallerBuilder>("127.0.0.1", 2024);
            mango::Director director;
            director.setBuilder(builder);

            director.construct();

            rpc_caller = std::dynamic_pointer_cast<mango::Caller>(builder->getResult());

            rpc_caller->Connect("127.0.0.1", 2012);
        }

    public:
        static std::shared_ptr<mango::Caller> rpc_caller;

        static void start()
        {
            if (!isRunning.exchange(true))
            {
                mango::MessageCreator::registerMessageType(8515, createTestMessage);
                mango::MessageCreator::registerMessageType(422050215, createFastMessage);
                mango::MessageCreator::registerMessageType(942936070, createSlowMessage);

                createService();
                createCaller();

                fut = std::async(std::launch::async, []
                                 { loquat::Epoll::GetInstance()->Wait(); });
            }
        }

        static void stop()
        {
            if (isRunning.exchange(false))
            {
                loquat::Epoll::GetInstance()->Terminate();
                fut.get();
            }
        }
    };

    std::atomic<bool> SharedControl::isRunning(false);
    std::future<void> SharedControl::fut;
    std::shared_ptr<mango::AsyncExecutorService> SharedControl::rpc_service;
    std::shared_ptr<mango::Caller> SharedControl::rpc_caller;

    class RpcTest : public ::testing::Test
    {
    public:
        void testCast()
        {
            TestMessage request;
            request.setMessage("Happy to see you, Emmy~");
            SharedControl::rpc_caller->cast(request);
        }

        void testCall()
        {
            TestMessage request;
            request.setMessage("Happy to see you, Emmy~");
            auto reply = SharedControl::rpc_caller->call(request);
            auto message = std::dynamic_pointer_cast<TestMessage>(reply);
            EXPECT_NE(message, nullptr);
            EXPECT_EQ(message->getMessage(), "Good to see you, Emma~");
        }

        void testAsync()
        {
            std::queue<std::string> queue;

            std::future<void> fut_1 = std::async(std::launch::async, [&queue]()
                                                 {
                                SlowMessage slow;
                                auto reply = SharedControl::rpc_caller->call(slow);
                                auto message = std::dynamic_pointer_cast<TestMessage>(reply);
                                queue.push(message->getMessage());
                                EXPECT_NE(message, nullptr);
                                EXPECT_EQ(message->getMessage(), "I'm June!"); });

            std::future<void> fut_2 = std::async(std::launch::async, [&queue]()
                                                 {
                                FastMessage slow;
                                auto reply = SharedControl::rpc_caller->call(slow);
                                auto message = std::dynamic_pointer_cast<TestMessage>(reply);
                                queue.push(message->getMessage());
                                EXPECT_NE(message, nullptr);
                                EXPECT_EQ(message->getMessage(), "I'm Lili!"); });
            fut_2.wait();
            fut_1.wait();
            auto msg = queue.front();
            EXPECT_EQ(msg, "I'm Lili!");
            queue.pop();msg = queue.front();
            EXPECT_EQ(msg, "I'm June!");
        }

        void testCast1K()
        {
            TestMessage request;
            request.setMessage("Happy to see you, Emmy~");
            for (int i = 0; i < 1024; i++)
            {
                SharedControl::rpc_caller->cast(request);
            }
        }

        void testCall100()
        {
            TestMessage request;
            request.setMessage("Happy to see you, Emmy~");
            for (int i = 0; i < 100; i++)
            {
                auto reply = SharedControl::rpc_caller->call(request);
                auto message = std::dynamic_pointer_cast<TestMessage>(reply);
                EXPECT_NE(message, nullptr);
                EXPECT_EQ(message->getMessage(), "Good to see you, Emma~");
            }
        }

        void testEnd()
        {
            SharedControl::stop();
        }

    protected:
        void SetUp() override
        {
            SharedControl::start();
        }

        void TearDown() override
        {
        }
    };

    TEST_F(RpcTest, testCast)
    {
        testCast();
    }

    TEST_F(RpcTest, testCall)
    {
        testCall();
    }

    TEST_F(RpcTest, testAsync)
    {
        testAsync();
    }

    TEST_F(RpcTest, testCast1K)
    {
        testCast1K();
    }

    TEST_F(RpcTest, testCall100)
    {
        testCall100();
    }

    TEST_F(RpcTest, testEnd)
    {
        testEnd();
    }
}
