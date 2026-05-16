#include <gtest/gtest.h>

#include "active.h"
#include "move.h"
#include "exit.h"
#include "protocol.h"
#include "app.h"

TEST(ActiveTest, SetActiveWhenNonActive) {
    App app;
    Active cmd(app, Status::ACTIVE);
    cmd.execute();
    EXPECT_EQ(app.getStatus(), Status::ACTIVE);
}

TEST(ActiveTest, SetSameStatusDoesNothing) {
    App app;
    app.setStatus(Status::ACTIVE);
    Active cmd(app, Status::ACTIVE);
    cmd.execute();
    EXPECT_EQ(app.getStatus(), Status::ACTIVE);
}

TEST(MoveTest, UpdateLocation) {
    App app;
    app.setStatus(Status::ACTIVE);
    std::vector<float>& locRef{app.getDeviceLocation()};
    locRef = {0, 0, 0};

    std::vector<float> newLoc = {5.0f, 6.0f, 7.0f};
    Move cmd(app, newLoc);
    EXPECT_NO_THROW(cmd.execute());
    ASSERT_EQ(locRef.size(), 3);
    EXPECT_FLOAT_EQ(locRef[0], 5.0f);
    EXPECT_FLOAT_EQ(locRef[1], 6.0f);
    EXPECT_FLOAT_EQ(locRef[2], 7.0f);
}

TEST(ProtocolTest, ChangeProtocolToBinary) {
    App app;
    Protocol cmd(app, TypeOfProtocol::BINARY);
    cmd.execute();
    EXPECT_EQ(app.getProtocol(), TypeOfProtocol::BINARY);
}

TEST(ProtocolTest, SameProtocolToJson) {
    App app;
    Protocol cmd(app, TypeOfProtocol::JSON);
    cmd.execute();
    EXPECT_EQ(app.getProtocol(), TypeOfProtocol::JSON);
}

TEST(ExitTest, SetsNotWorking) {
    App app;
    Exit cmd(app);
    cmd.execute();
    EXPECT_FALSE(app.isWorking());
}
