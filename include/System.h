//
// Created by Wande on 2/25/2021.
//

#ifndef D3PP_SYSTEM_H
#define D3PP_SYSTEM_H
#include <string>

#include "TaskScheduler.h"
#include "json.hpp"

using json = nlohmann::json;

const std::string SYSTEM_FILE_NAME = "System";
const float SYSTEM_VERSION_NUMBER = 0.02;

class System : TaskItem {
public:
    static bool IsRunning;
    static time_t startTime;
    static System* GetInstance();
    static System* Instance_;
    static std::mutex mainMutex;
    static std::string ServerName;
    std::string Motd;
    int ClickDistance;
    System();
private:
    bool SaveFile;
    time_t lastModified;

    void Load();
    void Save();
    void MainFunc();
};
#endif //D3PP_SYSTEM_H
