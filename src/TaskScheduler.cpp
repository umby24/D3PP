//
// Created by unknown on 2/18/2021.
//

#include "TaskScheduler.h"
const std::string MODULE_NAME = "Task Scheduler";
std::map<std::string, TaskItem> TaskScheduler::_tasks;
std::mutex TaskScheduler::_taskLock;

std::string TaskScheduler::RegisterTask(std::string name, TaskItem &item) {
    _taskLock.lock();
    auto i = _tasks.find(name);

    if (i != _tasks.end()) {
        // -- Not found :(
        //std::cout << "Attempt to register existing task: " << name << std::endl;
        Logger::LogAdd(MODULE_NAME, "Attempt to register existing task: " + name, WARNING, __FILE__, __LINE__, __FUNCTION__);
        auto postfix = rand() % 100 + 1;
        _taskLock.unlock();
        std::string finalName = RegisterTask(name + stringulate(postfix), item);
        return finalName;
    }

    _tasks.insert(std::make_pair(name, item));
    //std::cout << "Registered task: " << name << std::endl;
    Logger::LogAdd(MODULE_NAME, "Registered Task: " + name, NORMAL, __FILE__, __LINE__, __FUNCTION__);
    _taskLock.unlock();
    return name;
}

void TaskScheduler::UnregisterTask(std::string name) {
    _taskLock.lock();
    auto i = _tasks.find(name);

    if (i == _tasks.end()) {
        // -- Not found :(
        Logger::LogAdd(MODULE_NAME, "Attempt to unregister non-existing task: " + name, WARNING, __FILE__, __LINE__, __FUNCTION__);
        _taskLock.unlock();
        return;
    }

    _tasks.erase(i->first);
    _taskLock.unlock();
}

void TaskScheduler::RunSetupTasks() {
    std::map<std::string, TaskItem>::iterator it;
    _taskLock.lock();
    for (it = _tasks.begin(); it != _tasks.end(); it++) {
        try {
            if (it->second.Setup) {
                it->second.Setup();
            }
        } catch (int ex) {
           Logger::LogAdd(MODULE_NAME, "Error occurred in setup tasks: " + stringulate(ex), static_cast<LogType>(10), __FILE__, __LINE__, __FUNCTION__);
        }
    }
    _taskLock.unlock();
}

void TaskScheduler::RunMainTasks() {
    std::map<std::string, TaskItem>::iterator it;
    _taskLock.lock();
    for (it = _tasks.begin(); it != _tasks.end(); it++) {
        std::chrono::duration<double> time = (std::chrono::system_clock::now()-it->second.LastRun);
        if (time < it->second.Interval)
            continue;

        try {
            if (it->second.Main) {
                _taskLock.unlock();
                it->second.Main();
                _taskLock.lock();
                it->second.LastRun = std::chrono::system_clock::now();
            }
        } catch (int ex) {
            Logger::LogAdd(MODULE_NAME, "Error occurred in main tasks: " + stringulate(ex), static_cast<LogType>(10), __FILE__, __LINE__, __FUNCTION__);
        }
    }
    _taskLock.unlock();
}

void TaskScheduler::RunTeardownTasks() {
    std::map<std::string, TaskItem>::iterator it;
    _taskLock.lock();
    for (it = _tasks.begin(); it != _tasks.end(); it++) {
        try {
            if (it->second.Teardown) {
                it->second.Teardown();
            }
        } catch (int ex) {
            Logger::LogAdd(MODULE_NAME, "Error occurred in Teardown tasks: " + stringulate(ex),
                           static_cast<LogType>(10), __FILE__, __LINE__, __FUNCTION__);
        }
    }
    _taskLock.unlock();
}
