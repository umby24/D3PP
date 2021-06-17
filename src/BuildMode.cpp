#include "BuildMode.h"

BuildModeMain* BuildModeMain::Instance = nullptr;

const std::string MODULE_NAME = "BuildMode";

BuildModeMain::BuildModeMain() {
    SaveFile = false;
    hasLoaded = false;

    this->Interval = std::chrono::seconds(1);
    this->Setup = [this] { Load(); };
    this->Main = [this] { MainFunc(); };

    TaskScheduler::RegisterTask(MODULE_NAME, *this);
}

BuildModeMain* BuildModeMain::GetInstance() {
    if (Instance == nullptr) {
        Instance = new BuildModeMain();
    }

    return Instance;
}

void BuildModeMain::Load() {
    Files* f = Files::GetInstance();
    std::string filePath = f->GetFile(BUILD_MODE_FILE_NAME);
    
    PreferenceLoader pl(filePath, "");
    pl.LoadFile();

    _buildmodes.clear();

    for (auto const &item : pl.SettingsDictionary) {
        if (item.first.empty() || item.second.size() == 0)
            continue;

        pl.SelectGroup(item.first);

        BuildMode newBm;
        newBm.ID = item.first;
        newBm.Name = pl.Read("Name", "-");
        newBm.Plugin = pl.Read("Lua_Function", "");

        _buildmodes.insert(std::make_pair(item.first, newBm));
    }

    SaveFile = true;
    hasLoaded = true;
    LastFileDate = Utils::FileModTime(filePath);
    Logger::LogAdd(MODULE_NAME, "File loaded [" + filePath + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void BuildModeMain::Save() {
    Files* f = Files::GetInstance();
    std::string filePath = f->GetFile(BUILD_MODE_FILE_NAME);
    
    PreferenceLoader pl(filePath, "");
    
    for (auto const &item : _buildmodes) {
        pl.SelectGroup(item.first);
        pl.Write("Name", item.second.Name);
        pl.Write("Lua_Function", item.second.Plugin);
    }

    pl.SaveFile();
    LastFileDate = Utils::FileModTime(filePath);
    Logger::LogAdd(MODULE_NAME, "File saved [" + filePath + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void BuildModeMain::MainFunc() {

}

void BuildModeMain::Distribute(int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z, bool mode, unsigned char blockType) {

}

void BuildModeMain::Resend(int clientId) {

}

void BuildModeMain::SetMode(int clientId, std::string mode) {

}
