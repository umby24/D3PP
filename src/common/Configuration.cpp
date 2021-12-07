#include "common/Configuration.h"
#include "json.hpp"
#include "common/Files.h"
#include "common/Logger.h"
#include "Utils.h"
#include <fstream>
#define GLF __FILE__, __LINE__, __FUNCTION__

using json = nlohmann::json;
NetworkSettings Configuration::NetSettings { 32, 25565, true, false};
GeneralSettings Configuration::GenSettings { "D3PP Server", "Welcome to D3PP!","&cWelcome to D3PP", "INFO", 1,160, 3, true };
KillSettings Configuration::killSettings { 1, MinecraftLocation{ 0, 0, 0, 0, 0} };
Configuration* Configuration::_instance = nullptr;

Configuration* Configuration::GetInstance() {
    if (_instance == nullptr)
        _instance = new Configuration();
    
    return _instance;
}

Configuration::Configuration() {
    this->Interval = std::chrono::seconds(1);
    this->Main = [this] { MainFunc(); };
    this->Setup = [this] { Load(); };
    this->Teardown = [this] { Save(); };

    TaskScheduler::RegisterTask("Configuration", *this);
}

void Configuration::Load() {
    Files* fm = Files::GetInstance();
    std::string filePath = fm->GetFile("configuration");

    if (Utils::FileSize(filePath) == -1) {
        Save();
    }

    json j;
    std::ifstream inFile(filePath);
    try {
        inFile >> j;
         Configuration::NetSettings.LoadFromJson(j);
        Configuration::GenSettings.LoadFromJson(j);
        Configuration::killSettings.LoadFromJson(j);
    } catch (int Exception) {
        Logger::LogAdd("Configuration", "Error loading config file! using defaults.", LogType::L_ERROR, GLF);
    }

    inFile.close();

    Configuration::NetSettings.LoadFromJson(j);
    Configuration::GenSettings.LoadFromJson(j);
    Configuration::killSettings.LoadFromJson(j);
    Logger::LogAdd("Configuration", "Configuration Loaded.", LogType::NORMAL, GLF);

    lastLoaded = Utils::FileModTime(filePath);
}

void Configuration::Save() {
    Files* fm = Files::GetInstance();
    std::string filePath = fm->GetFile("configuration");
    json j;

    Configuration::NetSettings.SaveToJson(j);
    Configuration::GenSettings.SaveToJson(j);
    Configuration::killSettings.SaveToJson(j);

    std::ofstream outFile(filePath);
    outFile << std::setw(4) << j;
    outFile.flush();
    outFile.close();

    Logger::LogAdd("Configuration", "Configuration Saved.", LogType::NORMAL, GLF);
    lastLoaded = Utils::FileModTime(filePath);
}

void Configuration::MainFunc() {
    if (saveFile) {
        Save();
        saveFile = false;
    }

    Files* fm = Files::GetInstance();
    std::string filePath = fm->GetFile("configuration");
    time_t modTime =  Utils::FileModTime(filePath);
    
    if (modTime != lastLoaded) {
        Load();
    }
}