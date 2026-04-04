#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::Expectation;
using ::testing::Return;

class MockTimerClient : public TimerClient {
public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class MockTimedDoor : public TimedDoor {
public:
    MockTimedDoor(int timeout) : TimedDoor(timeout) {}
    MOCK_METHOD(void, throwState, (), (override));
    MOCK_METHOD(bool, isDoorOpened, (), (override));
    MOCK_METHOD(void, unlock, (), (override));
    MOCK_METHOD(void, lock, (), (override));
};

class TimedDoorTest : public ::testing::Test {
protected:
    void SetUp() override {
        door = new TimedDoor(5);
    }
    void TearDown() override {
        delete door;
    }
    TimedDoor* door;
};

TEST_F(TimedDoorTest, InitialStateIsClosed) {
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, LockUnlockChangesState) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, GetTimeOutReturnsCorrectValue) {
    EXPECT_EQ(door->getTimeOut(), 5);
}

TEST(DoorTimerAdapterTest, TimeoutCallsThrowState) {
    MockTimedDoor mockDoor(1);
    EXPECT_CALL(mockDoor, throwState()).Times(1);
    DoorTimerAdapter adapter(mockDoor);
    adapter.Timeout();
}

TEST(TimerTest, TregisterCallsTimeoutOnClient) {
    MockTimerClient mockClient;
    Timer timer;
    EXPECT_CALL(mockClient, Timeout()).Times(1);
    timer.tregister(0, &mockClient);
}

TEST(TimedDoorThrowStateTest, ThrowsWhenDoorOpened) {
    TimedDoor door(1);
    door.unlock();
    EXPECT_THROW(door.throwState(), std::runtime_error);
}

TEST(TimedDoorThrowStateTest, DoesNotThrowWhenDoorClosed) {
    TimedDoor door(1);
    EXPECT_NO_THROW(door.throwState());
}

TEST(TimedDoorIntegrationTest, UnlockThrowsOnTimeoutWhenDoorRemainsOpen) {
    TimedDoor door(0);
    EXPECT_THROW(door.unlock(), std::runtime_error);
}

TEST(TimedDoorIntegrationTest, UnlockDoesNotThrowIfLockedBeforeTimeout) {
    TimedDoor door(1);
    std::thread locker([&door]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        door.lock();
    });
    EXPECT_NO_THROW(door.unlock());
    locker.join();
}

TEST(TimedDoorIntegrationTest, UnlockThrowsWhenDoorNotClosedAfterTimeout) {
    TimedDoor door(0);
    EXPECT_THROW(door.unlock(), std::runtime_error);
}
