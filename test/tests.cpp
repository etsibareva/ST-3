// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <stdexcept>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::Mock;

class MockTimer : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor2 : public Door {
 public:
  MOCK_METHOD(void, lock, (), (override));
  MOCK_METHOD(void, unlock, (), (override));
  MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class DoorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    door = new TimedDoor(2);
  }

  void TearDown() override {
    delete door;
  }

  TimedDoor* door;
};

class AdapterTest : public ::testing::Test {
 protected:
  void SetUp() override {
        td = new TimedDoor(3);
        adap = new DoorTimerAdapter(*td);
  }

  void TearDown() override {
    delete adap;
    delete td;
  }

  TimedDoor* td;
  DoorTimerAdapter* adap;
};

class TimerTest2 : public ::testing::Test {
 protected:
  void SetUp() override {
        tim = new Timer();
        mClient = new MockTimer();
  }

  void TearDown() override {
    delete tim;
    delete mClient;
  }

  Timer* tim;
  MockTimer* mClient;
};

TEST_F(DoorTest, t1) {
  EXPECT_EQ(door->getTimeOut(), 2);
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(DoorTest, t2) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(DoorTest, t3) {
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(DoorTest, t4) {
  EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(AdapterTest, t5) {
  td->unlock();
  EXPECT_TRUE(td->isDoorOpened());
  EXPECT_THROW(adap->Timeout(), std::runtime_error);
}

TEST_F(AdapterTest, t6) {
  td->lock();
  EXPECT_FALSE(td->isDoorOpened());
  EXPECT_NO_THROW(adap->Timeout());
}

TEST_F(TimerTest2, t7) {
  EXPECT_CALL(*mClient, Timeout()).Times(1);
  tim->tregister(0, mClient);
}

TEST_F(TimerTest2, t8) {
  EXPECT_NO_THROW(tim->tregister(1, nullptr));
}

TEST_F(DoorTest, t9) {
  Timer tm;
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());

  DoorTimerAdapter ad(*door);

  EXPECT_THROW({
    tm.tregister(door->getTimeOut(), &ad);
  }, std::runtime_error);
}

TEST_F(DoorTest, t10) {
  Timer tm;
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());

  DoorTimerAdapter ad(*door);

  EXPECT_NO_THROW({
    tm.tregister(door->getTimeOut(), &ad);
  });
}

TEST_F(DoorTest, t11) {
  for (int i = 0; i < 5; i++) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
  }
}

TEST_F(DoorTest, t12) {
  TimedDoor* d2 = new TimedDoor(10);
  EXPECT_EQ(d2->getTimeOut(), 10);
  delete d2;
}

TEST(MockDoorTest, t13) {
  MockDoor2 mockDoor;
  
  EXPECT_CALL(mockDoor, lock()).Times(Exactly(1));
  EXPECT_CALL(mockDoor, unlock()).Times(Exactly(1));
  EXPECT_CALL(mockDoor, isDoorOpened()).Times(AtLeast(2))
      .WillOnce(::testing::Return(false))
      .WillOnce(::testing::Return(true));
  
  mockDoor.lock();
  EXPECT_FALSE(mockDoor.isDoorOpened());
  mockDoor.unlock();
  EXPECT_TRUE(mockDoor.isDoorOpened());
}

TEST(MockDoorTest, t14) {
  MockDoor2 mockDoor;
  
  EXPECT_CALL(mockDoor, lock());
  EXPECT_CALL(mockDoor, isDoorOpened()).WillOnce(::testing::Return(false));
  EXPECT_CALL(mockDoor, unlock());
  EXPECT_CALL(mockDoor, isDoorOpened()).WillOnce(::testing::Return(true));
  
  mockDoor.lock();
  EXPECT_FALSE(mockDoor.isDoorOpened());
  mockDoor.unlock();
  EXPECT_TRUE(mockDoor.isDoorOpened());
}
