//
// Created by Wande on 1/12/2022.
//

#include "world/MapActions.h"
#include "Utils.h"

D3PP::world::MapActions::MapActions() {
    Interval = std::chrono::milliseconds(30);
    Main = [this] { this->MainFunc(); };

    taskId = TaskScheduler::RegisterTask("HCMapActions" + stringulate(Utils::RandomNumber(193876957)), *this);
}

void D3PP::world::MapActions::MainFunc() {
    if (!itemQueue.empty()) {
        std::function <void()> taskToComplete = itemQueue.front();
        taskToComplete();
        itemQueue.pop();
    }
}

void D3PP::world::MapActions::AddTask(const std::function<void()>& task) {
    itemQueue.push(task);
}

D3PP::world::MapActions::~MapActions() {
    TaskScheduler::UnregisterTask(taskId);
}