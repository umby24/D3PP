#include <string>
#include <gtest/gtest.h>
#include "world/Entity.h"
#include "EventSystem.h"
#include "events/EventEntityAdd.h"

TEST(EntityTest, EntityMapChange) {
    bool hasDespawnHappened;
    bool hasRespawnHappened;

    Entity entityMonitoring("monitor", 0, 20, 20, 20, 10, 10);
    Entity entityTesting("test", 1, 30, 30, 30, 10, 10);

    Dispatcher::subscribe("derpy", [this, &hasDespawnHappened](auto && PH1){hasDespawnHappened = true;});
}

TEST(EntityTest, EntityPositionChange) {

}

TEST(EntityTest, EntitySpawns) {

}

TEST(EntityTest, EntityDespawns) {

}