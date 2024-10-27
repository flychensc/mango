#include <memory>

#include "caller_builder.h"
#include "executor_builder.h"
#include "director.h"
#include "message_creator.h"

#include "gtest/gtest.h"

namespace
{
    static bool s_init_flag;

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

    class RpcTest : public ::testing::Test
    {
    public:
        void testCast()
        {
            TestMessage request;
            request.setMessage("Happy to see you, Emmy~");
            rpc_caller_->cast(request);
        }

        void testCall()
        {
            TestMessage request;
            request.setMessage("Happy to see you, Emmy~");
            auto reply = rpc_caller_->call(request);
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
                rpc_caller_->cast(request);
            }
        }

        void testCall1K()
        {
            TestMessage request;
            request.setMessage("Happy to see you, Emmy~");
            for (int i = 0; i < 1024; i++)
            {
                auto reply = rpc_caller_->call(request);
                auto message = std::dynamic_pointer_cast<TestMessage>(reply);
                EXPECT_NE(message, nullptr);
                EXPECT_EQ(message->getMessage(), "Good to see you, Emma~");
            }
        }

    protected:
        void SetUp() override
        {
            if (s_init_flag)
            {
                mango::MessageCreator::registerMessageType(8515, createTestMessage);
                createService();
                createCaller();
            }
            s_init_flag = true;
        }

        void TearDown() override
        {
            rpc_caller_->stop();
            rpc_service_->stop();
        }

    private:
        std::shared_ptr<mango::ExecutorService> rpc_service_;
        std::shared_ptr<mango::Caller> rpc_caller_;

        void createService()
        {
            std::shared_ptr<mango::Builder> builder = std::make_shared<mango::ExecutorServiceBuilder>("127.0.0.1", 2012);
            mango::Director director;
            director.setBuilder(builder);

            director.construct();

            rpc_service_ = std::dynamic_pointer_cast<mango::ExecutorService>(builder->getResult());

            rpc_service_->start();
        }

        void createCaller()
        {
            std::shared_ptr<mango::Builder> builder = std::make_shared<mango::CallerBuilder>("127.0.0.1", 2024);
            mango::Director director;
            director.setBuilder(builder);

            director.construct();

            rpc_caller_ = std::dynamic_pointer_cast<mango::Caller>(builder->getResult());

            rpc_caller_->start();

            rpc_caller_->Connect("127.0.0.1", 2012);
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
    }

    TEST_F(RpcTest, testCall1K)
    {
    }
}
