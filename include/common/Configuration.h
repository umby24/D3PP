#ifndef D3PP_CONFIGURATION_H
#define D3PP_CONFIGURATION_H

#include <string>
#include "TaskScheduler.h"

struct GeneralSettings {
    std::string name;
    std::string motd;
    std::string logLevel;
    int ClickDistance;
    int LogPrune;
    bool LogArguments;
};

struct NetworkSettings {
    int MaxPlayers;
    int ListenPort;
    bool VerifyNames;
    bool Public;
};

class Configuration : public TaskItem {
public:
    static NetworkSettings NetSettings;
    static GeneralSettings GenSettings;

    Configuration();
private:

    void Load();
    void Save();
    void MainFunc();
};

#endif