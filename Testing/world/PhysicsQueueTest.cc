#include <gtest/gtest.h>
#include "common/Vectors.h"
#include "world/TimeQueueItem.h"
#include "world/PhysicsQueue.h"

TEST(PhysicsQueueTest, DeQueueEmpty) {
    D3PP::Common::Vector3S myThing {(short)10, 10, 10};
    D3PP::world::PhysicsQueue testing(myThing);
    D3PP::world::TimeQueueItem givenQueueItem{};

    bool actualResult = testing.TryDequeue(givenQueueItem);
    ASSERT_FALSE(actualResult);
    ASSERT_EQ(givenQueueItem.Location.X, 0);
}

TEST(PhysicsQueueTest, DequeueTest) {
    D3PP::Common::Vector3S myThing {(short)10, 10, 10};
    D3PP::world::PhysicsQueue testing(myThing);

    D3PP::world::TimeQueueItem givenQueueItem;
    givenQueueItem.Time = std::chrono::steady_clock::now();
    givenQueueItem.Location = D3PP::Common::Vector3S {(short)1, 0, 0};

    testing.TryQueue(givenQueueItem);
    D3PP::world::TimeQueueItem outQueueItem;
    bool actualResult = testing.TryDequeue(outQueueItem);

    ASSERT_TRUE(actualResult);
    ASSERT_EQ(outQueueItem.Location.X, 1);
}

TEST(PhysicsQueueTest, QueueDuplicate) {
    D3PP::Common::Vector3S givenQueueSize {(short)128, 128, 128};
    D3PP::world::PhysicsQueue testing(givenQueueSize);

    D3PP::world::TimeQueueItem givenQueueItem;
    givenQueueItem.Time = std::chrono::steady_clock::now()--;
    givenQueueItem.Location = D3PP::Common::Vector3S {(short)78,115,69};

    D3PP::world::TimeQueueItem givenDupeQueueItem;
    givenDupeQueueItem.Time = std::chrono::steady_clock::now()++;
    givenDupeQueueItem.Location = D3PP::Common::Vector3S {(short)78,115,69};

    testing.TryQueue(givenQueueItem);
    testing.TryQueue(givenDupeQueueItem);

    D3PP::world::TimeQueueItem outQueueItem;
    bool actualResult = testing.TryDequeue(outQueueItem);

    ASSERT_TRUE(actualResult);
    ASSERT_EQ(outQueueItem.Location.X, 78);

    bool secondResult = testing.TryDequeue(outQueueItem);
    ASSERT_FALSE(secondResult);
}