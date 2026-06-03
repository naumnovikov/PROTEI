#include <gtest/gtest.h>

#include "networkaddress.h"

TEST(NetworkAddressTest, DefaultConstructor)
{
    NetworkAddress test;
    EXPECT_EQ(test.getIp(), "192.168.0.102");
    EXPECT_EQ(test.getPort(), 2000);
}

TEST(NetworkAddressTest, StringConstructorValid)
{
    NetworkAddress test("192.168.1.100", "8080");
    EXPECT_EQ(test.getIp(), "192.168.1.100");
    EXPECT_EQ(test.getPort(), 8080);
}

TEST(NetworkAddressTest, StringConstructorInvalidPortThrows)
{
    EXPECT_THROW(NetworkAddress("192.168.1.100", "not_a_number"),
        std::invalid_argument);
}

TEST(NetworkAddressTest, TwoStringConstructorWithValid)
{
    NetworkAddress test("10.0.0.1", "443");
    EXPECT_EQ(test.getIp(), "10.0.0.1");
    EXPECT_EQ(test.getPort(), 443);
}

TEST(NetworkAddressTest, FullAddressStringConstructorValid)
{
    NetworkAddress test("10.0.0.1:25565");
    EXPECT_EQ(test.getIp(), "10.0.0.1");
    EXPECT_EQ(test.getPort(), 25565);
}

TEST(NetworkAddressTest, LittleEndianConstructorValid)
{
    // 0xC0A80164 in little‑endian = IP 192.168.1.100
    NetworkAddress test(0xC0A80164, 443);
    EXPECT_EQ(test.getIp(), "192.168.1.100");
    EXPECT_EQ(test.getPort(), 443);
}

TEST(NetworkAddressTest, LittleEndianConstructorZeroIP)
{
    NetworkAddress test(0, 1234);
    EXPECT_EQ(test.getIp(), "0.0.0.0");
    EXPECT_EQ(test.getPort(), 1234);
}

TEST(NetworkAddressTest, LittleEndianConstructorAllOnes)
{
    // 0xFFFFFFFF = 255.255.255.255
    NetworkAddress test(0xFFFFFFFF, 9999);
    EXPECT_EQ(test.getIp(), "255.255.255.255");
    EXPECT_EQ(test.getPort(), 9999);
}

TEST(NetworkAddressTest, PortBoundaryValues)
{
    NetworkAddress test1("1.2.3.4", 0);
    EXPECT_EQ(test1.getPort(), 0);
    NetworkAddress test2("1.2.3.4", 65535);
    EXPECT_EQ(test2.getPort(), 65535);
}

TEST(NetworkAddressTest, CopyAndAssign)
{
    NetworkAddress original("192.168.0.1", 8080);
    NetworkAddress copy(original);
    EXPECT_EQ(copy.getIp(), original.getIp());
    EXPECT_EQ(copy.getPort(), original.getPort());

    NetworkAddress assigned;
    assigned = original;
    EXPECT_EQ(assigned.getIp(), original.getIp());
    EXPECT_EQ(assigned.getPort(), original.getPort());
}