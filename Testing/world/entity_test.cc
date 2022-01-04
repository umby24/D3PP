#include <string>
#include <gtest/gtest.h>
#include "world/Entity.h"
#include "EventSystem.h"
#include "events/EventEntityAdd.h"
#include "events/EventEntityDelete.h"

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