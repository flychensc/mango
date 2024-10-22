#include "util.h"

#include <sys/socket.h>

#include "gtest/gtest.h"
namespace
{
    TEST(UTIL, determineDomain)
    {
        EXPECT_EQ(mango::determineDomain("1.1.1.1"), AF_INET);
        EXPECT_EQ(mango::determineDomain("10.90.90.1"), AF_INET);
        EXPECT_EQ(mango::determineDomain("192.168.1.1"), AF_INET);
        EXPECT_EQ(mango::determineDomain("127.0.0.1"), AF_INET);
        ASSERT_THROW(mango::determineDomain("1.2.3"), std::invalid_argument);
        ASSERT_THROW(mango::determineDomain("1111.2.3.4"), std::invalid_argument);
        ASSERT_THROW(mango::determineDomain("1.2222.3.4"), std::invalid_argument);
        ASSERT_THROW(mango::determineDomain("1.2.3333.4"), std::invalid_argument);
        ASSERT_THROW(mango::determineDomain("1.2.3.4444"), std::invalid_argument);
        ASSERT_THROW(mango::determineDomain("1.2.3.4.5"), std::invalid_argument);

        EXPECT_EQ(mango::determineDomain("::1"), AF_INET6);
        EXPECT_EQ(mango::determineDomain("fe80::02"), AF_INET6);
        EXPECT_EQ(mango::determineDomain("FE80::01"), AF_INET6);
        EXPECT_EQ(mango::determineDomain("2000::2017"), AF_INET6);
        EXPECT_EQ(mango::determineDomain("2134::2023"), AF_INET6);
        EXPECT_EQ(mango::determineDomain("1111:2222:3333:4444:5555:6666:7777:8888"), AF_INET6);
        EXPECT_EQ(mango::determineDomain("FF::"), AF_INET6);
        EXPECT_EQ(mango::determineDomain("::"), AF_INET6);
        ASSERT_THROW(mango::determineDomain("2134::2::1"), std::invalid_argument);

        EXPECT_EQ(mango::determineDomain("/tmp"), AF_UNIX);
        EXPECT_EQ(mango::determineDomain("/home/june"), AF_UNIX);
        ASSERT_THROW(mango::determineDomain("//tmp"), std::invalid_argument);
    }
}
