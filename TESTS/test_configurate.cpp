#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>

#include "app.h"
#include "server.h"

class ConfigurateTest : public ::testing::Test {
protected:
    App app;
    Server server;
    JSONParser parser;
    std::string tempFilename;

    inline void createTempJson(const std::string& content)
    {
        tempFilename = "/tmp/protei_test_config.json";
        std::ofstream out(tempFilename);
        EXPECT_TRUE(out.is_open()) << "Cannot create temporary file for test";
        out << content;
        out.close();
    }

    inline void TearDown() override { std::remove(tempFilename.c_str()); }
};

TEST_F(ConfigurateTest, ValidConfigSetAllFieldsInApp)
{
    std::string json { R"({
        "ip": "10.20.30.1",
        "imei": "123411234112341",
        "imsi": "123411234112341",
        "location": [55.75, 37.61, 23],
        "config": "active",
        "nodes": "node1,node2",
        "server_ip" : "192.168.0.102",
        "server_port" : 2048
    })" };
    createTempJson(json);
    EXPECT_NO_THROW(parser.configurateApp(tempFilename, app));

    EXPECT_EQ(app.getDeviceSocketIP(), "192.168.0.102");
    EXPECT_EQ(app.getDeviceSocketPort(), 2048);
    EXPECT_EQ(app.getDeviceIMEI(), "123411234112341");
    EXPECT_EQ(app.getDeviceIMSI(), "123411234112341");
    
    auto loc = app.getDeviceLocation();
    ASSERT_EQ(loc.size(), 3);
    EXPECT_FLOAT_EQ(loc[0], 55.75f);
    EXPECT_FLOAT_EQ(loc[1], 37.61f);
    EXPECT_FLOAT_EQ(loc[2], 23.0f);
    
    EXPECT_EQ(app.getDeviceConfig(), "active");
    EXPECT_EQ(app.getDeviceNodes(), "node1,node2");
}

TEST_F(ConfigurateTest, NoValidIMEI)
{
    std::string json { R"({
        "ip": "10.20.30.1",
        "imei": "1234",
        "imsi": "123456789012345",
        "location": [55.75, 37.61, 23.0],
        "server_ip" : "192.168.0.102",
        "server_port" : 2000
    })" };
    createTempJson(json);
    EXPECT_THROW(parser.configurateApp(tempFilename, app), std::invalid_argument);
}

TEST_F(ConfigurateTest, NoValidIMSI)
{
    std::string json { R"({
        "ip": "10.20.30.1",
        "imei": "111111111111111",
        "imsi": "1234567890123456", 
        "location": [55.75, 37.61, 23.0],
        "server_ip" : "192.168.0.102",
        "server_port" : 2000
    })" };
    createTempJson(json);
    EXPECT_THROW(parser.configurateApp(tempFilename, app), std::invalid_argument);
}

TEST_F(ConfigurateTest, NoValidLocation)
{
    std::string json { R"({
        "ip": "10.20.30.1",
        "imei": "111111111111111",
        "imsi": "111111111111111",
        "location": [55.75, 37.61],
        "server_ip" : "192.168.0.102",
        "server_port" : 2000
    })" };
    createTempJson(json);
    EXPECT_THROW(parser.configurateApp(tempFilename, app), std::invalid_argument);
}

TEST_F(ConfigurateTest, MissingRequiredFieldsThrows)
{
    std::string json { R"({"server_port": 8080})" };
    createTempJson(json);
    EXPECT_THROW(parser.configurateApp(tempFilename, app), std::invalid_argument);
}

TEST_F(ConfigurateTest, InvalidLastByteThrows)
{
    std::string json { R"({
        "ip": "10.0.0.0",
        "imei": "111111111111111",
        "imsi": "111111111111111",
        "location": [0,0,0],
        "server_ip" : "192.168.0.102",
        "server_port" : 2000
    })" };
    createTempJson(json);
    EXPECT_THROW(parser.configurateApp(tempFilename, app), std::invalid_argument);
}

TEST_F(ConfigurateTest, PortOutOfRangeThrows)
{
    std::string json { R"({
        "ip": "10.0.0.1",
        "imei": "111111111111111",
        "imsi": "111111111111111",
        "location": [0,0,0],
        "server_ip" : "192.168.0.102",
        "server_port" : 80
    })" };
    createTempJson(json);
    EXPECT_THROW(parser.configurateApp(tempFilename, app), std::invalid_argument);
}

TEST_F(ConfigurateTest, ValidMinPortAndIP)
{
    std::string json { R"({
        "ip": "192.168.0.1",
        "imei": "111111111111111",
        "imsi": "111111111111111",
        "location": [0,0,0],
        "server_ip" : "192.168.0.1",
        "server_port" : 1024
    })" };
    createTempJson(json);
    EXPECT_NO_THROW(parser.configurateApp(tempFilename, app));
    EXPECT_EQ(app.getDeviceSocketPort(), 1024);
    EXPECT_EQ(app.getDeviceSocketIP(), "192.168.0.1");
}

TEST_F(ConfigurateTest, ValidMaxPortAndIPLastByte253)
{
    std::string json { R"({
        "ip": "10.20.30.253",
        "imei": "111111111111111",
        "imsi": "111111111111111",
        "location": [0,0,0],
        "server_ip" : "10.20.30.253",
        "server_port" : 49151
    })" };
    createTempJson(json);
    EXPECT_NO_THROW(parser.configurateApp(tempFilename, app));
    EXPECT_EQ(app.getDeviceSocketPort(), 49151);
    EXPECT_EQ(app.getDeviceSocketIP(), "10.20.30.253");
}

TEST_F(ConfigurateTest, ValidIMSIminSize)
{
    std::string json { R"({
        "ip": "10.20.30.1",
        "imei": "111111111111111",
        "imsi": "1",
        "location": [0,0,0],
        "server_ip" : "192.168.0.102",
        "server_port" : 2000
    })" };
    createTempJson(json);
    EXPECT_NO_THROW(parser.configurateApp(tempFilename, app));
    EXPECT_EQ(app.getDeviceIMSI(), "1"); 
}

TEST_F(ConfigurateTest, InvalidJsonSyntax)
{
    std::string json { R"({ "ip": "10.0.0.1", "server_port": 2048, )" };
    createTempJson(json);
    EXPECT_THROW(parser.configurateApp(tempFilename, app), std::invalid_argument);
}

TEST_F(ConfigurateTest, FileNotFound)
{
    tempFilename = "/tmp/nonexistent_file_12345.json";
    EXPECT_THROW(parser.configurateApp(tempFilename, app), std::invalid_argument);
}

TEST_F(ConfigurateTest, ValidConfigSetAllFieldsInServer)
{
    std::string json { R"({
        "ip": "192.168.0.105",
        "port": 2048,
        "position": [55.75, 37.61, 23]
    })" };
    createTempJson(json);
    EXPECT_NO_THROW(parser.configurateServer(tempFilename, server));

    EXPECT_EQ(server.getPort(), 2048);
    EXPECT_EQ(server.getIp(), "192.168.0.105");
    ASSERT_EQ(server.getPosition().size(), 3);
    EXPECT_FLOAT_EQ(server.getPosition()[0], 55.75f);
    EXPECT_FLOAT_EQ(server.getPosition()[1], 37.61f);
    EXPECT_FLOAT_EQ(server.getPosition()[2], 23.0f);
}

TEST_F(ConfigurateTest, ServerInvalidIP)
{
    std::string json { R"({
        "ip": "10.0.0.0",
        "port": 2048,
        "position": [1,2,3]
    })" };
    createTempJson(json);
    EXPECT_THROW(parser.configurateServer(tempFilename, server),
        std::invalid_argument);
}

TEST_F(ConfigurateTest, ServerPortOutOfRange)
{
    std::string json { R"({
        "ip": "10.0.0.1",
        "port": 80,
        "position": [1,2,3]
    })" };
    createTempJson(json);
    EXPECT_THROW(parser.configurateServer(tempFilename, server),
        std::invalid_argument);
}

TEST_F(ConfigurateTest, ServerInvalidPositionSize)
{
    std::string json { R"({
        "ip": "10.0.0.1",
        "port": 2048,
        "position": [1,2]
    })" };
    createTempJson(json);
    EXPECT_THROW(parser.configurateServer(tempFilename, server),
        std::invalid_argument);
}

TEST_F(ConfigurateTest, ServerMissingRequiredFields)
{
    std::string json { R"({"port": 8080})" };
    createTempJson(json);
    EXPECT_THROW(parser.configurateServer(tempFilename, server),
        std::invalid_argument);
}

TEST_F(ConfigurateTest, ServerValidMinPortAndIP)
{
    std::string json { R"({
        "ip": "192.168.0.1",
        "port": 1024,
        "position": [0,0,0]
    })" };
    createTempJson(json);
    EXPECT_NO_THROW(parser.configurateServer(tempFilename, server));
    EXPECT_EQ(server.getPort(), 1024);
    EXPECT_EQ(server.getIp(), "192.168.0.1");
    ASSERT_EQ(server.getPosition().size(), 3);
}

TEST_F(ConfigurateTest, ServerValidMaxPortAndIPLastByte253)
{
    std::string json { R"({
        "ip": "10.20.30.253",
        "port": 49151,
        "position": [0,0,0]
    })" };
    createTempJson(json);
    EXPECT_NO_THROW(parser.configurateServer(tempFilename, server));
    EXPECT_EQ(server.getPort(), 49151);
    EXPECT_EQ(server.getIp(), "10.20.30.253");
}

TEST_F(ConfigurateTest, ServerInvalidJsonSyntax)
{
    std::string json { R"({ "ip": "10.0.0.1", "port": 2048 )" };
    createTempJson(json);
    EXPECT_THROW(parser.configurateServer(tempFilename, server),
        std::invalid_argument);
}