//
// Created by unknown on 2/18/2021.
//

#ifndef D3PP_TASKSCHEDULER_H
#define D3PP_TASKSCHEDULER_H

#include <string>
#include <map>
#include <chrono>
#include <mutex>
#include <functional>

class TaskItem {
public:
    TaskItem() {};
    std::string TaskId;
    std::chrono::duration<double> Interval;
    std::chrono::time_point<std::chrono::system_clock> LastRun;

    std::function<void()>Setup;
    std::function<void()>Main;
    std::function<void()>Teardown;
};

class TaskScheduler {
public:
    static std::string RegisterTask(std::string name, TaskItem &item);
    static void UnregisterTask(std::string name);
    static void RunSetupTasks();
    static void RunMainTasks();
    static void RunTeardownTasks();
private:
    static std::map<std::string, TaskItem> _tasks;
    static std::mutex _taskLock;
};

#endif //D3PP_TASKSCHEDULER_H
