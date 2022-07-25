#include <string>
#include <gtest/gtest.h>
#include "world/Entity.h"
#include "EventSystem.h"

TEST(EntityTest, EntityMapChange) {
    // bool hasDespawnHappened;
    // bool hasRespawnHappened;

    // Entity entityMonitoring("monitor", 0, 20, 20, 20, 10, 10);
    // Entity entityTesting("test", 1, 30, 30, 30, 10, 10);

    // Dispatcher::subscribe(EventEntityAdd::descriptor, [this, &hasDespawnHappened](auto && PH1){hasDespawnHappened = true;});
    // Dispatcher::subscribe(EventEntityDelete::descriptor, [this, &hasRespawnHappened](auto && PH1){hasRespawnHappened = true;});

    // entityMonitoring.MapID = 0;
    // entityTesting.MapID = 0;
    // entityTesting.PositionSet(1, MinecraftLocation{}, 0, false);
    // assert(hasDespawnHappened && hasRespawnHappened);
}

TEST(EntityTest, EntityPositionChange) {

}

TEST(EntityTest, EntitySpawns) {

}

TEST(EntityTest, EntityDespawns) {

}

TEST(EntityTest, GetPointerTest) {
    auto ePointer = Entity::GetPointer(2);
    assert(ePointer == nullptr);

    auto newEPointer = std::make_shared<Entity>("testEntity", 0, 1, 2, 3, 4, 5);
    Entity::Add(newEPointer);

    auto extPointer = Entity::GetPointer(0);
    assert(extPointer == newEPointer);

    auto nothingPointer = Entity::GetPointer(0, true);
    assert(nothingPointer == nullptr);
}
TEST(EntityTest, GetFreeIdTest) {
    auto newEPointer = std::make_shared<Entity>("testEntity", 0, 1, 2, 3, 4, 5);
    Entity::Add(newEPointer);

    ASSERT_EQ(newEPointer->Id, 0);
    ASSERT_EQ(Entity::GetFreeId(), 1);
}

TEST(EntityTest, SetDisplayNameTest) {
    Entity::SetDisplayName(123, "givenPrefix", "givenName", "givenSuffix");
    auto newEPointer = std::make_shared<Entity>("testEntity", 0, 1, 2, 3, 4, 5);
    Entity::Add(newEPointer);

    Entity::SetDisplayName(0, "givenPrefix", "givenName", "givenSuffix");
    ASSERT_EQ(newEPointer->Suffix,"givenSuffix");
    ASSERT_EQ(newEPointer->Name,"givenName");
    ASSERT_EQ(newEPointer->Prefix, "givenPrefix");
}