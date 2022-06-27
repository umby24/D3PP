//
// Created by Wande on 1/12/2022.
//

#ifndef D3PP_MAPINTENSIVEACTIONS_H
#define D3PP_MAPINTENSIVEACTIONS_H
#include <queue>
#include <thread>
#include <functional>

namespace D3PP::world {
    class MapIntensiveActions {
    public:
        MapIntensiveActions();
        ~MapIntensiveActions();
        void Terminate();
        void MainFunc();
        void AddTask(const std::function<void()>& task);
    private:
        std::queue<std::function<void()>> itemQueue;
        std::thread runner;
        bool m_finished;
    };
}
#endif //D3PP_MAPINTENSIVEACTIONS_H
