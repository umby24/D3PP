//
// Created by unknown on 2/18/2021.
//

#include "common/TaskScheduler.h"

#include <ranges>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "Utils.h"
#include "common/Logger.h"
#include "common/Files.h"

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
    Logger::LogAdd("TaskScheduler", "Marked for deletion", DEBUG, GLF);
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
    auto it = _tasks.begin();
    while (it != _tasks.end()) {
        std::chrono::duration<double> time = (std::chrono::system_clock::now() - it->second.LastRun);
        if (time < it->second.Interval) {
            ++it;
            continue;
        }

        try {
            if (it->second.Main) {
                auto start_time = std::chrono::high_resolution_clock::now();
                it->second.Main();
                auto end_time = std::chrono::high_resolution_clock::now();
                
                auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                
                // Update execution statistics
                it->second.LastExecutionTime = execution_time;
                it->second.TotalExecutionTime += execution_time;
                it->second.ExecutionCount++;
                
                if (execution_time > it->second.MaxExecutionTime) {
                    it->second.MaxExecutionTime = execution_time;
                }
                if (execution_time < it->second.MinExecutionTime) {
                    it->second.MinExecutionTime = execution_time;
                }
                
                it->second.LastRun = std::chrono::system_clock::now();
                if (it->second.IsDeleted) {
                    Logger::LogAdd(MODULE_NAME, "Unregistered Task: " + it->first, VERBOSE, GLF);
                    it = _tasks.erase(it);
                    continue;
                }
            }
        } catch (int ex) {
            Logger::LogAdd(MODULE_NAME, "Error occurred in main tasks: " + stringulate(ex), static_cast<LogType>(10), GLF);
        }
        ++it;
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

void TaskScheduler::GenerateTaskMonitoringReport() {
    const std::scoped_lock<std::mutex> lock(_taskLock);
    
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream html;
    html << R"(<!DOCTYPE html>
<html>
<head>
    <title>D3PP Task Scheduler Monitor</title>
    <style type="text/css">
        body {
            font-family: "Microsoft PhagsPa", Arial, sans-serif;
            color: #2f2f2f;
            background-color: #F7F7F7;
            margin: 0;
            padding: 20px;
        }
        h1.header {
            background-color: darkblue;
            text-shadow: 2px 1px 0px rgba(0,0,0,.2);
            font-size: 25px;
            font-weight: bold;
            text-decoration: none;
            text-align: center;
            color: white;
            margin: 0;
            padding: 15px;
            margin-bottom: 20px;
            border-radius: 5px;
        }
        table {
            border: 1px solid #A0A0A0;
            table-layout: auto;
            empty-cells: hide;
            border-collapse: collapse;
            width: 100%;
            margin-bottom: 20px;
        }
        tr {
            border: 1px solid #A0A0A0;
            background-color: #D0D0D0;
            color: #212121;
        }
        tr:nth-child(even) {
            background-color: #E0E0E0;
        }
        th {
            background-color: #B0B0B0;
            font-weight: bold;
            padding: 10px;
            border-right: 1px solid #A0A0A0;
            text-align: left;
        }
        td {
            padding: 8px;
            border-right: 1px solid #A0A0A0;
        }
        .number {
            text-align: right;
        }
        .good-time {
            color: #00AA00;
            font-weight: bold;
        }
        .warning-time {
            color: #AA5500;
            font-weight: bold;
        }
        .bad-time {
            color: #AA0000;
            font-weight: bold;
        }
        .footer {
            text-align: center;
            margin-top: 20px;
            color: #666;
        }
    </style>
    <meta http-equiv="refresh" content="30">
</head>
<body>
    <h1 class="header">D3PP Task Scheduler Monitor</h1>
    
    <table>
        <tr>
            <th>Task Name</th>
            <th>Interval (s)</th>
            <th>Executions</th>
            <th>Last Execution (ms)</th>
            <th>Min Time (ms)</th>
            <th>Max Time (ms)</th>
            <th>Average Time (ms)</th>
            <th>Total Time (ms)</th>
            <th>Status</th>
        </tr>
)";

    for (const auto& [name, task] : _tasks) {
        if (task.IsDeleted) continue;
        
        double interval_seconds = task.Interval.count();
        long long avg_time = task.ExecutionCount > 0 ? task.TotalExecutionTime.count() / task.ExecutionCount : 0;
        
        // Determine time class based on execution time
        std::string time_class = "good-time";
        if (task.LastExecutionTime.count() > 100) {
            time_class = "warning-time";
        }
        if (task.LastExecutionTime.count() > 500) {
            time_class = "bad-time";
        }
        
        std::string status = "Active";
        auto time_since_last = std::chrono::duration_cast<std::chrono::seconds>(now - task.LastRun);
        if (time_since_last.count() > interval_seconds * 2) {
            status = "Delayed";
        }
        
        html << "        <tr>\n";
        html << "            <td>" << name << "</td>\n";
        html << "            <td class=\"number\">" << std::fixed << std::setprecision(1) << interval_seconds << "</td>\n";
        html << "            <td class=\"number\">" << task.ExecutionCount << "</td>\n";
        html << "            <td class=\"number " << time_class << "\">" << task.LastExecutionTime.count() << "</td>\n";
        html << "            <td class=\"number\">" << (task.MinExecutionTime.count() == 999999 ? 0 : task.MinExecutionTime.count()) << "</td>\n";
        html << "            <td class=\"number\">" << task.MaxExecutionTime.count() << "</td>\n";
        html << "            <td class=\"number\">" << avg_time << "</td>\n";
        html << "            <td class=\"number\">" << task.TotalExecutionTime.count() << "</td>\n";
        html << "            <td>" << status << "</td>\n";
        html << "        </tr>\n";
    }
    
    html << R"(    </table>
    
    <div class="footer">
        Report generated at )" << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S") << R"(<br>
        Auto-refresh every 30 seconds
    </div>
</body>
</html>)";

    try {
        std::string filename = Files::GetFile("TaskScheduler_Monitor");
        std::ofstream file(filename);
        if (file.is_open()) {
            file << html.str();
            file.close();
            Logger::LogAdd(MODULE_NAME, "Task monitoring report generated: " + filename, VERBOSE, GLF);
        } else {
            Logger::LogAdd(MODULE_NAME, "Failed to open file for task monitoring report: " + filename, WARNING, GLF);
        }
    } catch (const std::exception& e) {
        Logger::LogAdd(MODULE_NAME, "Exception while generating task monitoring report: " + std::string(e.what()), WARNING, GLF);
    }
}

void TaskScheduler::InitializeMonitoringTask() {
    TaskItem monitoringTask;
    monitoringTask.Interval = std::chrono::seconds(30);
    monitoringTask.LastRun = std::chrono::system_clock::now();
    monitoringTask.Main = []() {
        GenerateTaskMonitoringReport();
    };
    
    RegisterTask("TaskScheduler_Monitor", monitoringTask);
    Logger::LogAdd(MODULE_NAME, "Task monitoring system initialized - reports will be generated every 30 seconds", NORMAL, GLF);
}
