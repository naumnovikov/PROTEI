#include <gtest/gtest.h>

#include "socketBusinessWorker.h"

TEST(SocketWorkerTest, DecodeFloatFromBEBytes)
{
    SocketBusinessWorker sw;

    // 12.31f (big-endian bytes)
    uint8_t bytes1[] { 0x41, 0x44, 0xF5, 0xC3 };
    EXPECT_FLOAT_EQ(sw.decodeFloatFromBEBytes(bytes1), 12.31f);

    // 0.0f
    uint8_t zero[] { 0x00, 0x00, 0x00, 0x00 };
    EXPECT_FLOAT_EQ(sw.decodeFloatFromBEBytes(zero), 0.0f);

    // -314.0f
    uint8_t neg[] { 0xC3, 0x9D, 0x00, 0x00 };
    EXPECT_FLOAT_EQ(sw.decodeFloatFromBEBytes(neg), -314.0f);
}

TEST(SocketWorkerTest, FillMsgInBinaryFormatInBE)
{
    SocketBusinessWorker sw;
    std::vector<float> loc { 12.31f, -314.0f, 31.12f };
    std::vector<uint8_t> msg;
    sw.fillMsgInBinaryFormatInBE(msg, loc);

    const uint8_t expected[] {
        0x00, 0x00, 0x00, 0x0D, // rest_len = 13
        0x01, // binary flag
        0x41, 0x44, 0xF5, 0xC3, // 12.31
        0xC3, 0x9D, 0x00, 0x00, // -314.0
        0x41, 0xF8, 0xF5, 0xC3 // 31.12
    };
    ASSERT_EQ(msg.size(), sizeof(expected));
    EXPECT_EQ(memcmp(msg.data(), expected, sizeof(expected)), 0);
}

TEST(SocketWorkerTest, FillMsgInJSONFormatInBE)
{
    SocketBusinessWorker sw;
    std::vector<float> loc { 12.31f, -314.0f, 31.12f };
    std::vector<uint8_t> msg;
    sw.fillMsgInJSONFormatInBE(msg, loc);

    ASSERT_GE(msg.size(), 5);
    uint32_t rest_len { static_cast<uint32_t>((msg[0] << 24) | (msg[1] << 16) | (msg[2] << 8) | msg[3]) };
    EXPECT_EQ(msg[4], JSONTypeByte);
    std::string json_str(msg.begin() + 5, msg.end());
    EXPECT_EQ(rest_len, protocol_len + json_str.size());
    EXPECT_TRUE(json_str.find("\"location\"") != std::string::npos);
}