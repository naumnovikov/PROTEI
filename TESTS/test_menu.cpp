#include <gtest/gtest.h>

#include "active.h"
#include "app.h"
#include "exit.h"
#include "move.h"
#include "protocol.h"

TEST(ActiveTest, SetActiveWhenNonActive) {
  App app;
  Active cmd(app, Status::ACTIVE);
  cmd.execute();
  EXPECT_EQ(app.getStatus(), Status::ACTIVE);
}

TEST(ActiveTest, SetNonActiveWhenActive) {
  App app;
  app.setStatus(Status::ACTIVE);
  Active cmd(app, Status::NON_ACTIVE);
  cmd.execute();
  EXPECT_EQ(app.getStatus(), Status::NON_ACTIVE);
}

TEST(ActiveTest, SetSameStatusDoesNothing) {
  App app;
  app.setStatus(Status::ACTIVE);
  Active cmd(app, Status::ACTIVE);
  cmd.execute();
  EXPECT_EQ(app.getStatus(), Status::ACTIVE);
}

TEST(ProtocolTest, ChangeProtocolToBinary) {
  App app;
  Protocol cmd(app, TypeOfProtocol::BINARY);
  cmd.execute();
  EXPECT_EQ(app.getProtocol(), TypeOfProtocol::BINARY);
}

TEST(ProtocolTest, ChangeProtocolToJson) {
  App app;
  Protocol cmd(app, TypeOfProtocol::JSON);
  cmd.execute();
  EXPECT_EQ(app.getProtocol(), TypeOfProtocol::JSON);
}

TEST(ProtocolTest, SameProtocolDoesNotChange) {
  App app;
  app.setProtocol(TypeOfProtocol::BINARY);
  Protocol cmd(app, TypeOfProtocol::BINARY);
  cmd.execute();
  EXPECT_EQ(app.getProtocol(), TypeOfProtocol::BINARY);
}

TEST(ExitTest, SetsNotWorking) {
  App app;
  Exit cmd(app);
  cmd.execute();
  EXPECT_FALSE(app.isWorking());
}

TEST(ExitTest, AlreadyNotWorkingDoesNothing) {
  App app;
  app.setAppWorkingState(WorkingState::NOT_WORKING);
  Exit cmd(app);
  cmd.execute();
  EXPECT_FALSE(app.isWorking());
}

TEST(AppStateTest, IsWorkingAfterActiveCommand) {
  App app;
  Active cmd(app, Status::ACTIVE);
  cmd.execute();
  EXPECT_TRUE(app.isWorking());
}

TEST(AppStateTest, IsWorkingAfterProtocolChange) {
  App app;
  Protocol cmd(app, TypeOfProtocol::BINARY);
  cmd.execute();
  EXPECT_TRUE(app.isWorking());
}