#include "app.h"

#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>

class ConfigurateTest : public ::testing::Test {
protected:
    App app;
    std::string tempFilename;

    void createTempJson(const std::string& content) {
        tempFilename = "/tmp/protei_test_config.json";
        std::ofstream out(tempFilename);
        EXPECT_TRUE(out.is_open()) << "Cannot create temporary file for test";
        out << content;
        out.close();
    }

    void TearDown() override {
        std::remove(tempFilename.c_str());
    }
};

TEST_F(ConfigurateTest, ValidConfigSetsAllFields) {
    std::string json = R"({
        "ip": "10.20.30.1",
        "port": 2048,
        "imei": [1, 2, 3, 4],
        "imsi": [5, 6, 7],
        "location": [55.75, 37.61],
        "config": "active",
        "nodes": "node1,node2"
    })";
    createTempJson(json);
    EXPECT_NO_THROW(app.configurate(tempFilename));

    const auto& dev{app.getDevice()};
    EXPECT_EQ(dev.socket.getIp(), "10.20.30.1");
    EXPECT_EQ(dev.socket.getPort(), 2048);
    EXPECT_EQ(dev.imei, (std::vector<char>{1, 2, 3, 4}));
    EXPECT_EQ(dev.imsi, (std::vector<char>{5, 6, 7}));
    ASSERT_EQ(dev.location.size(), 2);
    EXPECT_FLOAT_EQ(dev.location[0], 55.75f);
    EXPECT_FLOAT_EQ(dev.location[1], 37.61f);
    EXPECT_EQ(dev.config, "active");
    EXPECT_EQ(dev.nodes, "node1,node2");
}

TEST_F(ConfigurateTest, MissingRequiredFieldsThrows) {
    std::string json{R"({"port": 8080})"};
    createTempJson(json);
    EXPECT_THROW(app.configurate(tempFilename), std::invalid_argument);
}

TEST_F(ConfigurateTest, InvalidLastByteThrows) {
    std::string json{R"({"ip": "10.0.0.0", "port": 1024})"};
    createTempJson(json);
    EXPECT_THROW(app.configurate(tempFilename), std::invalid_argument);
}

TEST_F(ConfigurateTest, PortOutOfRangeThrows) {
    std::string json{R"({"ip": "10.0.0.1", "port": 80})"};
    createTempJson(json);
    EXPECT_THROW(app.configurate(tempFilename), std::invalid_argument);
}
