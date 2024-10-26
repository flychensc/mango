#include "message_creator.h"

#include "gtest/gtest.h"
namespace
{
    class TestMessage : public mango::Message
    {
    public:
        TestMessage()
        {
            Type = 0x12345678;
            setBody("TEST-MESSAGE");
        }

    private:
        void setBody(const std::string &str)
        {
            Message::setBody(std::vector<loquat::Byte>(str.begin(), str.end()));
        }
    };

    std::unique_ptr<mango::Message> createTestMessage()
    {
        return std::make_unique<TestMessage>();
    }

    TEST(Message, Serialize)
    {
        TestMessage message;
        auto data = message.Serialize();

        EXPECT_EQ(data[0], 0x12);
        EXPECT_EQ(data[1], 0x34);
        EXPECT_EQ(data[2], 0x56);
        EXPECT_EQ(data[3], 0x78);
        EXPECT_EQ(std::string(data.begin() + 4, data.end()), "TEST-MESSAGE");
    }

    TEST(Message, Deserialize)
    {
        TestMessage message;
        auto data = message.Serialize();

        mango::Message new_message;
        new_message.Deserialize(data);

        EXPECT_EQ(new_message.Type, message.Type);
        EXPECT_EQ(new_message.Serialize(), message.Serialize());
    }

    TEST(MessageCreator, Deserialize)
    {
        mango::MessageCreator::registerMessageType(0x12345678, createTestMessage);

        TestMessage message;
        auto data = message.Serialize();

        auto new_message = mango::MessageCreator::Deserialize(data);

        EXPECT_EQ(new_message->Type, message.Type);
        EXPECT_EQ(new_message->Serialize(), message.Serialize());
    }
}
