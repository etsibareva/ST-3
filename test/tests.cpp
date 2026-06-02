// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <stdexcept>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Exactly;

class MockTimerClient : public TimerClient {
public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class MockTimedDoor : public TimedDoor {
public:
    MockTimedDoor(int timeout) : TimedDoor(timeout) {}
    MOCK_METHOD(void, lock, (), (override));
    MOCK_METHOD(void, unlock, (), (override));
    MOCK_METHOD(bool, isDoorOpened, (), (override));
    MOCK_METHOD(void, throwState, (), (override));
};

class MockDoorTimerAdapter : public DoorTimerAdapter {
public:
    MockDoorTimerAdapter(TimedDoor& door) : DoorTimerAdapter(door) {}
    MOCK_METHOD(void, Timeout, (), (override));
};

class TestTimedDoorAll : public ::testing::Test {
protected:
    void SetUp() override {
        timedDoor = new TimedDoor(4);
        mockTimedDoor = new MockTimedDoor(4);
        doorTimedAdapter = new DoorTimerAdapter(*timedDoor);
        timer = new Timer();
        mockTimerClient = new MockTimerClient();
    }
    void TearDown() override {
        delete timedDoor;
        delete mockTimedDoor;
        delete doorTimedAdapter;
        delete timer;
        delete mockTimerClient;
    }
    TimedDoor* timedDoor;
    MockTimedDoor* mockTimedDoor;
    DoorTimerAdapter* doorTimedAdapter;
    Timer* timer;
    MockTimerClient* mockTimerClient;
};

TEST_F(TestTimedDoorAll, DoorInitCorrect) {
    EXPECT_EQ(timedDoor->getTimeOut(), 4);
    EXPECT_FALSE(timedDoor->isDoorOpened());
}

TEST_F(TestTimedDoorAll, UnlockOpensDoor) {
    timedDoor->unlock();
    EXPECT_TRUE(timedDoor->isDoorOpened());
}

TEST_F(TestTimedDoorAll, LockClosesDoor) {
    EXPECT_FALSE(timedDoor->isDoorOpened());
    timedDoor->unlock();
    EXPECT_TRUE(timedDoor->isDoorOpened());
    timedDoor->lock();
    EXPECT_FALSE(timedDoor->isDoorOpened());
}

TEST_F(TestTimedDoorAll, TimerRegisterCorrect) {
    timedDoor->lock();
    EXPECT_NO_THROW(timer->tregister(timedDoor->getTimeOut(), doorTimedAdapter));
}

TEST_F(TestTimedDoorAll, MockDoorUnlockOpensDoor) {
    EXPECT_CALL(*mockTimedDoor, unlock()).Times(Exactly(1));
    EXPECT_CALL(*mockTimedDoor, isDoorOpened()).WillOnce(Return(true));
    mockTimedDoor->unlock();
    EXPECT_TRUE(mockTimedDoor->isDoorOpened());
}

TEST_F(TestTimedDoorAll, MockDoorLockClosesDoor) {
    EXPECT_CALL(*mockTimedDoor, lock()).Times(Exactly(1));
    EXPECT_CALL(*mockTimedDoor, isDoorOpened()).WillOnce(Return(false));
    mockTimedDoor->lock();
    EXPECT_FALSE(mockTimedDoor->isDoorOpened());
}

TEST_F(TestTimedDoorAll, TimeoutThrowsWhenDoorOpenedTooLong) {
    timedDoor->unlock();
    EXPECT_THROW(timer->tregister(1, doorTimedAdapter), std::runtime_error);
}

TEST_F(TestTimedDoorAll, TimeoutNoThrowWhenDoorClosedTooLong) {
    timedDoor->lock();
    EXPECT_NO_THROW(timer->tregister(1, doorTimedAdapter));
}

TEST_F(TestTimedDoorAll, TimerCallsClientTimeout) {
    EXPECT_CALL(*mockTimerClient, Timeout()).Times(Exactly(1));
    timer->tregister(1, mockTimerClient);
}

TEST_F(TestTimedDoorAll, AdapterCallsDoorMethodsOnTimeout) {
    DoorTimerAdapter adapter(*mockTimedDoor);
    EXPECT_CALL(*mockTimedDoor, isDoorOpened()).WillOnce(Return(true));
    EXPECT_CALL(*mockTimedDoor, throwState()).Times(Exactly(1));
    EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST_F(TestTimedDoorAll, AdapterDoesNotCallThrowStateWhenDoorClosed) {
    DoorTimerAdapter adapter(*mockTimedDoor);
    
    EXPECT_CALL(*mockTimedDoor, isDoorOpened()).WillOnce(Return(false));
    EXPECT_CALL(*mockTimedDoor, throwState()).Times(Exactly(0));
    
    EXPECT_NO_THROW(adapter.Timeout());
}
