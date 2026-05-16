#include <gtest/gtest.h>

#include "app.h"

TEST(InputParsingTest, SimpleCommand) {
    App app;
    auto tokens{app.interpretateInputCommand("ACTIVE TRUE")};
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "ACTIVE");
    EXPECT_EQ(tokens[1], "TRUE");
}

TEST(InputParsingTest, ExtraSpaces) {
    App app;
    auto tokens{app.interpretateInputCommand("  MOVE  1.0  2.0  ")};
    ASSERT_EQ(tokens.size(), 3);
    EXPECT_EQ(tokens[0], "MOVE");
    EXPECT_EQ(tokens[1], "1.0");
    EXPECT_EQ(tokens[2], "2.0");
}

TEST(InputParsingTest, EmptyInput) {
    App app;
    auto tokens{app.interpretateInputCommand("   ")};
    EXPECT_TRUE(tokens.empty());
}

TEST(ProcessActiveTest, MissingArgumentThrows) {
    App app;
    std::vector<std::string> tokens = {"ACTIVE"};
    EXPECT_THROW(app.ProcessACTIVE(tokens), std::invalid_argument);
}

TEST(ProcessActiveTest, ValidTrue) {
    App app;
    std::vector<std::string> tokens = {"ACTIVE", "TRUE"};
    EXPECT_EQ(tokens.size(), 2);
    EXPECT_NO_THROW(app.ProcessACTIVE(tokens));
    EXPECT_EQ(app.getStatus(), Status::ACTIVE);
}

TEST(ProcessProtocolTest, InvalidArgumentThrows) {
    App app;
    std::vector<std::string> tokens = {"PROTOCOL", "INVALID"};
    EXPECT_THROW(app.ProcessPROTOCOL(tokens), std::invalid_argument);
}

TEST(ProcessMoveTest, NonActiveThrows) {
    App app;
    std::vector<std::string> tokens = {"MOVE", "1.0"};
    EXPECT_THROW(app.ProcessMOVE(tokens), std::logic_error);
}
