// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <stdexcept>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::Mock;

// Mock класс для тестирования TimerClient
class MockTimerClient : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override));
};

// Mock класс для тестирования Door
class MockDoor : public Door {
 public:
  MOCK_METHOD(void, lock, (), (override));
  MOCK_METHOD(void, unlock, (), (override));
  MOCK_METHOD(bool, isDoorOpened, (), (override));
};

// Тестовый класс для TimedDoor
class TimedDoorTestUnique : public ::testing::Test {
 protected:
  void SetUp() override {
    testDoor = new TimedDoor(2);
  }

  void TearDown() override {
    delete testDoor;
  }

  TimedDoor* testDoor;
};

// Тестовый класс для DoorTimerAdapter
class DoorTimerAdapterTestUnique : public ::testing::Test {
 protected:
  void SetUp() override {
    door = new TimedDoor(3);
    adapter = new DoorTimerAdapter(*door);
  }

  void TearDown() override {
    delete adapter;
    delete door;
  }

  TimedDoor* door;
  DoorTimerAdapter* adapter;
};

// Тестовый класс для Timer
class TimerTestUnique : public ::testing::Test {
 protected:
  void SetUp() override {
    timer = new Timer();
    mockClient = new MockTimerClient();
  }

  void TearDown() override {
    delete timer;
    delete mockClient;
  }

  Timer* timer;
  MockTimerClient* mockClient;
};

// Тест 1: Проверка создания двери с корректным таймаутом
TEST_F(TimedDoorTestUnique, ConstructorSetsCorrectTimeout) {
  EXPECT_EQ(testDoor->getTimeOut(), 2);
  EXPECT_FALSE(testDoor->isDoorOpened());
}

// Тест 2: Проверка метода lock() закрывает дверь
TEST_F(TimedDoorTestUnique, LockClosesDoor) {
  testDoor->unlock();
  EXPECT_TRUE(testDoor->isDoorOpened());
  testDoor->lock();
  EXPECT_FALSE(testDoor->isDoorOpened());
}

// Тест 3: Проверка метода unlock() открывает дверь
TEST_F(TimedDoorTestUnique, UnlockOpensDoor) {
  testDoor->lock();
  EXPECT_FALSE(testDoor->isDoorOpened());
  testDoor->unlock();
  EXPECT_TRUE(testDoor->isDoorOpened());
}

// Тест 4: Проверка метода throwState выбрасывает исключение
TEST_F(TimedDoorTestUnique, ThrowStateThrowsException) {
  EXPECT_THROW(testDoor->throwState(), std::runtime_error);
}

// Тест 5: Проверка DoorTimerAdapter вызывает throwState когда дверь открыта
TEST_F(DoorTimerAdapterTestUnique, TimeoutCallsThrowStateWhenDoorOpened) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
  EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

// Тест 6: Проверка DoorTimerAdapter не вызывает исключение когда дверь закрыта
TEST_F(DoorTimerAdapterTestUnique, TimeoutDoesNotThrowWhenDoorClosed) {
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
  EXPECT_NO_THROW(adapter->Timeout());
}

// Тест 7: Проверка Timer вызывает Timeout у клиента
TEST_F(TimerTestUnique, TimerCallsTimeoutOnClient) {
  EXPECT_CALL(*mockClient, Timeout()).Times(1);
  timer->tregister(0, mockClient);
}

// Тест 8: Проверка Timer с нулевым клиентом не вызывает ошибку
TEST_F(TimerTestUnique, TimerWithNullClientDoesNotCrash) {
  EXPECT_NO_THROW(timer->tregister(1, nullptr));
}

// Тест 9: Интеграционный тест - открытая дверь вызывает исключение
TEST_F(TimedDoorTestUnique, OpenDoorThrowsExceptionAfterTimeout) {
  Timer testTimer;
  testDoor->unlock();
  EXPECT_TRUE(testDoor->isDoorOpened());

  DoorTimerAdapter testAdapter(*testDoor);

  EXPECT_THROW({
    testTimer.tregister(testDoor->getTimeOut(), &testAdapter);
  }, std::runtime_error);
}

// Тест 10: Интеграционный тест - закрытая дверь без исключения
TEST_F(TimedDoorTestUnique, ClosedDoorDoesNotThrowExceptionAfterTimeout) {
  Timer testTimer;
  testDoor->lock();
  EXPECT_FALSE(testDoor->isDoorOpened());

  DoorTimerAdapter testAdapter(*testDoor);

  EXPECT_NO_THROW({
    testTimer.tregister(testDoor->getTimeOut(), &testAdapter);
  });
}

// Тест 11: Проверка многократного открытия/закрытия
TEST_F(TimedDoorTestUnique, MultipleOpenCloseCyclesWorkCorrectly) {
  for (int i = 0; i < 5; i++) {
    testDoor->unlock();
    EXPECT_TRUE(testDoor->isDoorOpened());
    testDoor->lock();
    EXPECT_FALSE(testDoor->isDoorOpened());
  }
}

// Тест 12: Проверка что getTimeOut возвращает правильное значение
TEST_F(TimedDoorTestUnique, GetTimeOutReturnsCorrectValue) {
  TimedDoor* anotherDoor = new TimedDoor(10);
  EXPECT_EQ(anotherDoor->getTimeOut(), 10);
  delete anotherDoor;
}
