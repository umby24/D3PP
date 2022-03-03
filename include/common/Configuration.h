#ifndef D3PP_CONFIGURATION_H
#define D3PP_CONFIGURATION_H

#include <string>
#include "TaskScheduler.h"
#include "json.hpp"
#include "Vectors.h"
#include "MinecraftLocation.h"

using json = nlohmann::json;
using namespace D3PP::Common;

struct TextSettings {
    std::string error;
    std::string system;
    std::string divider;

    void LoadFromJson(json &j) {
        if (j.is_object() && !j["Text"].is_null()) {
            error = j["Text"]["Error"];
            system = j["Text"]["System"];
            divider = j["Text"]["Divider"];
        }
    }

    void SaveToJson(json &j) {
        j["Text"] = nullptr;
        j["Text"]["Error"] = error;
        j["Text"]["System"] = system;
        j["Text"]["Divider"] = divider;
    }
};

struct GeneralSettings {
    std::string name;
    std::string motd;
    std::string WelcomeMessage;
    std::string logLevel;
    int SpawnMapId;
    int ClickDistance;
    int LogPrune;
    bool LogArguments;

    void LoadFromJson(json &j) {
        if (j.is_object() && !j["General"].is_null()) {
            name = j["General"]["Name"];
            motd = j["General"]["Motd"];
            logLevel = j["General"]["LogLevel"];
            ClickDistance = j["General"]["ClickDistance"];
            LogPrune = j["General"]["LogPrune"];
            LogArguments = j["General"]["LogArguments"];
            SpawnMapId = j["General"]["SpawnMapId"];
            WelcomeMessage = j["General"]["WelcomeMessage"];
        }
    }

    void SaveToJson(json &j) {
        j["General"] = nullptr;
        j["General"]["Name"] = name;
        j["General"]["Motd"] = motd;
        j["General"]["LogLevel"] = logLevel;
        j["General"]["ClickDistance"] = ClickDistance;
        j["General"]["LogPrune"] = LogPrune;
        j["General"]["LogArguments"] = LogArguments;
        j["General"]["SpawnMapId"] = SpawnMapId;
        j["General"]["WelcomeMessage"] = WelcomeMessage;
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

struct KillSettings {
    int killMapId;
    MinecraftLocation killSpawn;
    int killSpawnX;
    int killSpawnY;
    int killSpawnZ;
    int killSpawnRot;
    int killSpawnLook;

    void LoadFromJson(json &j) {
        if (j.is_object() && !j["Kill"].is_null()) {
            killMapId = j["Kill"]["mapId"];
            MinecraftLocation ks { j["Kill"]["rotation"].get<float>(), j["Kill"]["look"].get<float>() };
            Vector3S ksVec { j["Kill"]["x"].get<short>(), j["Kill"]["y"].get<short>(), j["Kill"]["z"].get<short>()};
            ks.SetAsBlockCoords(ksVec);
            killSpawn = ks;
        }
    }

    void SaveToJson(json &j) {
        j["Kill"] = nullptr;
        j["Kill"]["mapId"] = killMapId;
        j["Kill"]["rotation"] = killSpawn.Rotation;
        j["Kill"]["look"] = killSpawn.Look;
        Vector3S blockCoords = killSpawn.GetAsBlockCoords();
        j["Kill"]["x"] = blockCoords.X;
        j["Kill"]["y"] = blockCoords.Y;
        j["Kill"]["z"] = blockCoords.Z;
    }
};
class Configuration : public TaskItem {
public:
    static NetworkSettings NetSettings;
    static GeneralSettings GenSettings;
    static KillSettings killSettings;
    static TextSettings textSettings;
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