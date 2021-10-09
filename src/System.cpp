//
// Created by Wande on 2/25/2021.
//

#include "System.h"
#include <iomanip>
#include "common/Files.h"
#include "Utils.h"
#include "common/Logger.h"
#include "common/Configuration.h"

const std::string MODULE_NAME = "System";
System* System::Instance_ = nullptr;
bool System::IsRunning = false;
time_t System::startTime = time(nullptr);
std::mutex System::mainMutex;
std::string System::ServerName = "D3PP Server";

System::System() {
    this->Interval = std::chrono::seconds(2000);
    this->Setup = [this] { Load(); };
    this->Teardown = [this] {Save(); };

    // -- Default values
    System::ServerName = "D3PP Server";
    Motd = "&cWelcome to D3PP!";
    ClickDistance = 160;
    SaveFile = false;
    lastModified = 0;

    TaskScheduler::RegisterTask(MODULE_NAME, *this);
}

void System::Load() {
    System::ServerName = Configuration::GenSettings.name;
    Motd = Configuration::GenSettings.motd;
    ClickDistance = Configuration::GenSettings.ClickDistance;
}

void System::Save() {
    Configuration::GenSettings.name = System::ServerName;
    Configuration::GenSettings.motd = Motd;
    Configuration::GenSettings.ClickDistance = ClickDistance;
}


System* System::GetInstance() {
    if (Instance_ == nullptr)
        Instance_ = new System();
    
    return Instance_;
}