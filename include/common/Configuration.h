#ifndef D3PP_CONFIGURATION_H
#define D3PP_CONFIGURATION_H

#include <string>
#include "TaskScheduler.h"
#include "json.hpp"

using json = nlohmann::json;

struct GeneralSettings {
    std::string name;
    std::string motd;
    std::string logLevel;
    int ClickDistance;
    int LogPrune;
    bool LogArguments;

    void LoadFromJson(json j) {
        if (j.is_object() && !j["General"].is_null()) {
            name = j["General"]["Name"];
            motd = j["General"]["Motd"];
            logLevel = j["General"]["LogLevel"];
            ClickDistance = j["General"]["ClickDistance"];
            LogPrune = j["General"]["LogPrune"];
            LogArguments = j["General"]["LogArguments"];
        }
    }

    void SaveToJson(json j) {
        j["General"] = nullptr;
        j["General"]["Name"] = name;
        j["General"]["Motd"] = motd;
        j["General"]["LogLevel"] = logLevel;
        j["General"]["ClickDistance"] = ClickDistance;
        j["General"]["LogPrune"] = LogPrune;
        j["General"]["LogArguments"] = LogArguments;
    }
};

struct NetworkSettings {
    int MaxPlayers;
    int ListenPort;
    bool VerifyNames;
    bool Public;

    void LoadFromJson(json &j) {
        if (j.is_object() && !j["Network"].is_null()) {
            MaxPlayers = j["Network"]["MaxPlayers"];
            ListenPort = j["Network"]["ListenPort"];
            VerifyNames = j["Network"]["VerifyName"];
            Public = j["Network"]["Public"];
        }
    }

    void SaveToJson(json &j) {
        j["Network"] = nullptr;
        j["Network"]["MaxPlayers"] = MaxPlayers;
        j["Network"]["ListenPort"] = ListenPort;
        j["Network"]["VerifyName"] = VerifyNames;
        j["Network"]["Public"] = Public;
    }
};

class Configuration : public TaskItem {
public:
    static NetworkSettings NetSettings;
    static GeneralSettings GenSettings;

    Configuration();
    static Configuration* GetInstance();
private:
    static Configuration* _instance;
    time_t lastLoaded;
    bool saveFile;

    void Load();
    void Save();
    void MainFunc();
};

#endif