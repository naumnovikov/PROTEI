#include <fstream>
#include <cstdio>

#include <gtest/gtest.h>

#include "app.h"
#include "server.h"

class ConfigurateTest : public ::testing::Test {
protected:
    App app;
    Server server;
    JSONParser parser;
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

TEST_F(ConfigurateTest, ValidConfigSetAllFieldsInApp) {
    std::string json = R"({
        "ip": "10.20.30.1",
        "port": 2048,
        "imei": [1, 2, 3, 4, 1, 1, 2, 3, 4, 1, 1, 2, 3, 4, 1],
        "imsi": [1, 2, 3, 4, 1, 1, 2, 3, 4, 1, 1, 2, 3, 4, 1],
        "location": [55.75, 37.61, 23],
        "config": "active",
        "nodes": "node1,node2"
    })";
    createTempJson(json);
    EXPECT_NO_THROW(parser.configurateApp(tempFilename, app));

    const auto& dev{app.getDevice()};
    EXPECT_EQ(dev.socket.getIp(), "10.20.30.1");
    EXPECT_EQ(dev.socket.getPort(), 2048);
    EXPECT_EQ(dev.imei, (std::vector<char>{1, 2, 3, 4, 1, 1, 2, 3, 4, 1, 1, 2, 3, 4, 1}));
    EXPECT_EQ(dev.imsi, (std::vector<char>{1, 2, 3, 4, 1, 1, 2, 3, 4, 1, 1, 2, 3, 4, 1}));
    ASSERT_EQ(dev.location.size(), 3);
    EXPECT_FLOAT_EQ(dev.location[0], 55.75f);
    EXPECT_FLOAT_EQ(dev.location[1], 37.61f);
    EXPECT_FLOAT_EQ(dev.location[2], 23);
    EXPECT_EQ(dev.config, "active");
    EXPECT_EQ(dev.nodes, "node1,node2");
}

TEST_F(ConfigurateTest, ValidConfigSetAllFieldsInServer) {
    std::string json = R"({
        "port": 2048,
        "position": [55.75, 37.61, 23]
    })";
    createTempJson(json);
    JSONParser parser;
    EXPECT_NO_THROW(parser.configurateServer(tempFilename, server));

    EXPECT_EQ(server.getPort(), 2048);
    ASSERT_EQ(server.getPosition().size(), 3);
}

TEST_F(ConfigurateTest, NoValidIMEI) {
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
    EXPECT_THROW(parser.configurateApp(tempFilename, app), std::invalid_argument);
}

TEST_F(ConfigurateTest, NoValidIMSI) {
    std::string json = R"({
        "ip": "10.20.30.1",
        "port": 2048,
        "imei": [1, 2, 3, 4, 1, 1, 2, 3, 4, 1, 1, 2, 3, 4, 1],
        "imsi": [1, 2, 3, 4, 1, 1, 2, 3, 4, 1, 1, 2, 3, 4, 1, 2],
        "location": [55.75, 37.61],
        "config": "active",
        "nodes": "node1,node2"
    })";
    createTempJson(json);
    EXPECT_THROW(parser.configurateApp(tempFilename, app), std::invalid_argument);
}

TEST_F(ConfigurateTest, NoValidLocation) {
    std::string json = R"({
        "ip": "10.20.30.1",
        "port": 2048,
        "imei": [1, 2, 3, 4, 1, 1, 2, 3, 4, 1, 1, 2, 3, 4, 1],
        "imsi": [1, 2, 3, 4, 1, 1, 2, 3, 4, 1, 1, 2, 3, 4, 1],
        "location": [55.75, 37.61],
        "config": "active",
        "nodes": "node1,node2"
    })";
    createTempJson(json);
    EXPECT_THROW(parser.configurateApp(tempFilename, app), std::invalid_argument);
}

TEST_F(ConfigurateTest, MissingRequiredFieldsThrows) {
    std::string json{R"({"port": 8080})"};
    createTempJson(json);
    EXPECT_THROW(parser.configurateApp(tempFilename, app), std::invalid_argument);
}

TEST_F(ConfigurateTest, InvalidLastByteThrows) {
    std::string json{R"({"ip": "10.0.0.0", "port": 1024})"};
    createTempJson(json);
    EXPECT_THROW(parser.configurateApp(tempFilename, app), std::invalid_argument);
}

TEST_F(ConfigurateTest, PortOutOfRangeThrows) {
    std::string json{R"({"ip": "10.0.0.1", "port": 80})"};
    createTempJson(json);
    EXPECT_THROW(parser.configurateApp(tempFilename, app), std::invalid_argument);
}
