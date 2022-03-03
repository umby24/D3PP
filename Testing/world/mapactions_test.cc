#include <string>
#include <gtest/gtest.h>
#include "world/MapActions.h"

TEST(MapActions, TaskExecutes) {
    bool taskHasExecuted = false;
    D3PP::world::MapActions actions;
    std::function<void()> lambda = [&taskHasExecuted](){taskHasExecuted = true;};

    actions.AddTask(lambda);
    actions.MainFunc();

    assert(taskHasExecuted);
}