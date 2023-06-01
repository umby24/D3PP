#include "common/Configuration.h"

#include "common/Files.h"
#include "common/Logger.h"
#include "Utils.h"

#define GLF __FILE__, __LINE__, __FUNCTION__

NetworkSettings Configuration::NetSettings { 32, 25565, true, false};
GeneralSettings Configuration::GenSettings { "D3PP Server", "Welcome to D3PP!","&cWelcome to D3PP", "INFO", "default.cw", 0,160, 3, true };
KillSettings Configuration::killSettings { 1, MinecraftLocation{ 0, 0, Vector3S((short)0, (short)0, (short)0)} };
TextSettings Configuration::textSettings { "&4Error:&f ", "&e", "&3|" };
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
    saveFile = false;
    filepath = Files::GetFile("configuration");

    TaskScheduler::RegisterTask("Configuration", *this);
}

void Configuration::Load() {
    if (Utils::FileSize(filepath) == -1) {
        Save();
    }

    json j;
    std::ifstream inFile(filepath);
    
    if (!inFile.is_open()) {
        return;
    }

    try {
        inFile >> j;
         Configuration::NetSettings.LoadFromJson(j);
        Configuration::GenSettings.LoadFromJson(j);
        Configuration::killSettings.LoadFromJson(j);
        Configuration::textSettings.LoadFromJson(j);
    } catch (std::exception e) {
        Logger::LogAdd("Configuration", "Error loading config file! using defaults.", LogType::L_ERROR, GLF);
    }

    inFile.close();
    Logger::LogAdd("Configuration", "Configuration Loaded.", LogType::NORMAL, GLF);

    lastLoaded = Utils::FileModTime(filepath);
}

void Configuration::Save() {
    json j;

    Configuration::NetSettings.SaveToJson(j);
    Configuration::GenSettings.SaveToJson(j);
    Configuration::killSettings.SaveToJson(j);
    Configuration::textSettings.SaveToJson(j);

    std::ofstream outFile(filepath);
    outFile << std::setw(4) << j;
    outFile.flush();
    outFile.close();

    Logger::LogAdd("Configuration", "Configuration Saved.", LogType::NORMAL, GLF);
    lastLoaded = Utils::FileModTime(filepath);
}

void Configuration::MainFunc() {
    if (saveFile) {
        Save();
        saveFile = false;
    }

    time_t modTime =  Utils::FileModTime(filepath);
    
    if (modTime != lastLoaded) {
        Load();
    }
}