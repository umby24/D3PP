#include "world/MapIntensiveActions.h"
#include <functional>
#include <queue>

#include "System.h"
#include "Utils.h"

D3PP::world::MapIntensiveActions::MapIntensiveActions() : runner([this]() {this->MainFunc();}) {
}

void D3PP::world::MapIntensiveActions::MainFunc() {
    while (System::IsRunning) {
        if (!itemQueue.empty()) {
            std::function<void()> taskToComplete = itemQueue.front();
            taskToComplete();
            itemQueue.pop();
        }
    }
}

void D3PP::world::MapIntensiveActions::AddTask(const std::function<void()>& task) {
    itemQueue.push(task);
}

D3PP::world::MapIntensiveActions::~MapIntensiveActions() {
}