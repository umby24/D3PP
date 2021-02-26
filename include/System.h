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

class System : TaskItem {
public:
    std::string ServerName;
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
