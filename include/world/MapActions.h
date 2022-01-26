//
// Created by Wande on 1/12/2022.
//

#ifndef D3PP_MAPACTIONS_H
#define D3PP_MAPACTIONS_H
#include "common/TaskScheduler.h"
#include <queue>

namespace D3PP::world {
    class MapActions : public TaskItem {
    public:
        MapActions();
        void MainFunc();
        void AddTask(const std::function<void()>& task);
    private:
        std::queue<std::function<void()>> itemQueue;
    };
}
#endif //D3PP_MAPACTIONS_H
