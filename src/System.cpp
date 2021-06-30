//
// Created by Wande on 2/25/2021.
//

#include "System.h"
#include <iomanip>
#include "Files.h"
#include "Utils.h"
#include "Logger.h"

const std::string MODULE_NAME = "System";
System* System::Instance_ = nullptr;
bool System::IsRunning = false;

System::System() {
    this->Interval = std::chrono::seconds(2);
    this->Setup = [this] { Load(); };
    this->Main = [this] { MainFunc(); };
    this->Teardown = [this] {Save(); };

    // -- Default values
    ServerName = "D3PP Server";
    Motd = "&cWelcome to D3PP!";
    ClickDistance = 160;
    SaveFile = false;
    lastModified = 0;

    TaskScheduler::RegisterTask(MODULE_NAME, *this);
}

void System::Load() {
    Files *f = Files::GetInstance();
    std::string filePath = f->GetFile(SYSTEM_FILE_NAME);
    json j;

    std::ifstream iStream(filePath);

    if (!iStream.is_open()) {
        Logger::LogAdd(MODULE_NAME, "Failed to load", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        Save();
        return;
    }

    iStream >> j;
    iStream.close();

    ServerName = j["ServerName"];
    Motd = j["MOTD"];
    ClickDistance = j["ClickDistance"];

    Logger::LogAdd(MODULE_NAME, "File loaded.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    time_t modTime = Utils::FileModTime(filePath);
    lastModified = modTime;
}

void System::Save() {
    json j;
    Files* f = Files::GetInstance();
    std::string blockFile = f->GetFile(SYSTEM_FILE_NAME);

    j["ServerName"] = ServerName;
    j["MOTD"] = Motd;
    j["ClickDistance"] = ClickDistance;

    std::ofstream oStream(blockFile, std::ios::trunc);

    if (!oStream.is_open()) {
        Logger::LogAdd(MODULE_NAME, "Failed to save!", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    oStream << std::setw(4) << j;
    oStream.flush();
    oStream.close();

    time_t modTime = Utils::FileModTime(blockFile);
    lastModified = modTime;

    Logger::LogAdd(MODULE_NAME, "File saved.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void System::MainFunc() {
    if (SaveFile) {
        SaveFile = false;
        Save();
    }

    Files* f = Files::GetInstance();
    std::string blockFile = f->GetFile(SYSTEM_FILE_NAME);
    time_t modTime = Utils::FileModTime(blockFile);

    if (modTime != lastModified) {
        Load();
        lastModified = modTime;
    }
}

System* System::GetInstance() {
    if (Instance_ == nullptr)
        Instance_ = new System();
    
    return Instance_;
}