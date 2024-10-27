#include <atomic>
#include <memory>
#include <future>
#include <thread>

#include "caller_builder.h"
#include "executor_builder.h"
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

    class SharedControl
    {
    private:
        static std::atomic<bool> isRunning;
        static std::future<void> fut;

        static std::shared_ptr<mango::ExecutorService> rpc_service;

        static void createService()
        {
            std::shared_ptr<mango::Builder> builder = std::make_shared<mango::ExecutorServiceBuilder>("::1", 170504);
            mango::Director director;
            director.setBuilder(builder);

            director.construct();

            rpc_service = std::dynamic_pointer_cast<mango::ExecutorService>(builder->getResult());
        }

        static void createCaller()
        {
            std::shared_ptr<mango::Builder> builder = std::make_shared<mango::CallerBuilder>("::1", 230510);
            mango::Director director;
            director.setBuilder(builder);

            director.construct();

            rpc_caller = std::dynamic_pointer_cast<mango::Caller>(builder->getResult());

            rpc_caller->Connect("::1", 170504);
        }

    public:
        static std::shared_ptr<mango::Caller> rpc_caller;

        static void start()
        {
            if (!isRunning.exchange(true))
            {
                mango::MessageCreator::registerMessageType(8515, createTestMessage);
                createService();
                createCaller();

                fut = std::async(std::launch::async, []
                                 { loquat::Epoll::GetInstance().Wait(); });
            }
        }

        static void stop()
        {
            if (isRunning.exchange(false))
            {
                loquat::Epoll::GetInstance().Terminate();
                fut.get();
            }
        }
    };

    std::atomic<bool> SharedControl::isRunning(false);
    std::future<void> SharedControl::fut;
    std::shared_ptr<mango::ExecutorService> SharedControl::rpc_service;
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

        void testCast1K()
        {
            TestMessage request;
            request.setMessage("Happy to see you, Emmy~");
            for (int i = 0; i < 1024; i++)
            {
                SharedControl::rpc_caller->cast(request);
            }
        }

        void testCall1K()
        {
            TestMessage request;
            request.setMessage("Happy to see you, Emmy~");
            for (int i = 0; i < 1024; i++)
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

    TEST_F(RpcTest, testCast1K)
    {
        testCast1K();
    }

    TEST_F(RpcTest, testCall1K)
    {
        testCall1K();
    }

    TEST_F(RpcTest, testEnd)
    {
        testEnd();
    }
}
