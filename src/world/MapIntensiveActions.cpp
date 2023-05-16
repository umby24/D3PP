#include "world/MapIntensiveActions.h"
#include <functional>
#include <queue>

#include "System.h"
#include "Utils.h"

D3PP::world::MapIntensiveActions::MapIntensiveActions() : runner([this]() {this->MainFunc();}) {
    m_finished = false;
}

void D3PP::world::MapIntensiveActions::MainFunc() {
    while (!m_finished) {
        if (!itemQueue.empty()) {
            std::function<void()> taskToComplete = itemQueue.front();
            taskToComplete();
            itemQueue.pop();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void D3PP::world::MapIntensiveActions::AddTask(const std::function<void()>& task) {
    itemQueue.push(task);
}

D3PP::world::MapIntensiveActions::~MapIntensiveActions() {
    Terminate();
}

void D3PP::world::MapIntensiveActions::Terminate() {
    m_finished = true;
    runner.join();
}
