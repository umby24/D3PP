//
// Created by unknown on 2/18/2021.
//

#include "common/TaskScheduler.h"

#include <ranges>

#include "Utils.h"
#include "common/Logger.h"

const std::string MODULE_NAME = "Task Scheduler";
std::map<std::string, TaskItem> TaskScheduler::_tasks;
std::mutex TaskScheduler::_taskLock;

std::string TaskScheduler::RegisterTask(std::string name, TaskItem &item) {
    auto i = _tasks.find(name);

    if (i != _tasks.end()) {
        // -- Not found :(
        Logger::LogAdd(MODULE_NAME, "Attempt to register existing task: " + name, VERBOSE, GLF);
        auto postfix = rand() % 100 + 1;
        std::string finalName = RegisterTask(name + stringulate(postfix), item);
        item.TaskId = finalName;
        return finalName;
    }

    _tasks.insert(std::make_pair(name, item));
    Logger::LogAdd(MODULE_NAME, "Registered Task: " + name, VERBOSE, GLF);
    item.TaskId = name;
    return name;
}

void TaskScheduler::UnregisterTask(std::string name) {
    auto i = _tasks.find(name);

    if (i == _tasks.end()) {
        // -- Not found :(
        Logger::LogAdd(MODULE_NAME, "Attempt to unregister non-existing task: " + name, DEBUG, GLF);
     
        return;
    }
    i->second.IsDeleted = true;
}

void TaskScheduler::RunSetupTasks() {
    for (auto &val: _tasks | std::views::values) {
        try {
            if (val.Setup) {
                val.Setup();
            }
        } catch (int ex) {
           Logger::LogAdd(MODULE_NAME, "Error occurred in setup tasks: " + stringulate(ex), static_cast<LogType>(10), GLF);
        }
    }
}

void TaskScheduler::RunMainTasks() {
    for (auto &val: _tasks) {
        std::chrono::duration<double> time = (std::chrono::system_clock::now()-val.second.LastRun);
        if (time < val.second.Interval)
            continue;

        try {
            if (val.second.Main) {
                val.second.Main();
                val.second.LastRun = std::chrono::system_clock::now();
                if (val.second.IsDeleted) {
                    Logger::LogAdd(MODULE_NAME, "Unregistered Task: " + val.first, VERBOSE, GLF);
                    _tasks.erase(val.first);
                    break;
                }
            }
        } catch (int ex) {
            Logger::LogAdd(MODULE_NAME, "Error occurred in main tasks: " + stringulate(ex), static_cast<LogType>(10), GLF);
        }
    }
}

void TaskScheduler::RunTeardownTasks() {
    for (auto &val: _tasks | std::views::values) {
        try {
            if (val.Teardown) {
                val.Teardown();
            }
        } catch (int ex) {
            Logger::LogAdd(MODULE_NAME, "Error occurred in Teardown tasks: " + stringulate(ex),
                           static_cast<LogType>(10), GLF);
        }
    }
}
