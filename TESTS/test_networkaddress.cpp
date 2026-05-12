#include "../APP/app/networkaddress.h"

#include <gtest/gtest.h>

TEST(NetworkAddressTest, StringConstructorValid) {
    NetworkAddress test("192.168.1.100", "8080");
    EXPECT_EQ(test.getIp(), "192.168.1.100");
    EXPECT_EQ(test.getPort(), 8080);
}

TEST(NetworkAddressTest, LittleEndianConstructor) {
    // 0xC0A80164 = 192.168.1.100 в Little Endian
    NetworkAddress test(0xC0A80164, 443);
    EXPECT_EQ(test.getIp(), "192.168.1.100");
    EXPECT_EQ(test.getPort(), 443);
}

TEST(NetworkAddressTest, FullAddressStringConstructor) {
    NetworkAddress test("10.0.0.1 25565");
    EXPECT_EQ(test.getIp(), "10.0.0.1");
    EXPECT_EQ(test.getPort(), 25565);
}

TEST(NetworkAddressTest, DefaultConstructor) {
    NetworkAddress test;
    EXPECT_EQ(test.getIp(), "0.0.0.0");
    EXPECT_EQ(test.getPort(), 0);
}

TEST(NetworkAddressTest, EdgePortMin) {
    NetworkAddress test("1.2.3.4", "1");
    EXPECT_EQ(test.getPort(), 1);
}

TEST(NetworkAddressTest, LargePort) {
    NetworkAddress test("255.255.255.255", "65535");
    EXPECT_EQ(test.getPort(), 65535);
}
