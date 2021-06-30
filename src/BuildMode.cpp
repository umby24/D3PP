#include "BuildMode.h"

BuildModeMain* BuildModeMain::Instance = nullptr;

const std::string MODULE_NAME = "BuildMode";

BuildModeMain::BuildModeMain() {
    SaveFile = false;
    hasLoaded = false;
    LastFileDate = 0;

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
    if (SaveFile) {
        SaveFile = false;
        Save();
    }

    Files* f = Files::GetInstance();
    std::string bmFile = f->GetFile(BUILD_MODE_FILE_NAME);
    time_t modTime = Utils::FileModTime(bmFile);

    if (modTime != LastFileDate) {
        Load();
        LastFileDate = modTime;
    }

    while (_resendBlocks.size() > BUILD_MODE_BLOCKS_TO_RESEND_SIZE_MAX) {
        _resendBlocks.erase(_resendBlocks.end());
    }
}

void BuildModeMain::Distribute(int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z, bool mode, unsigned char blockType) {
    Network* n = Network::GetInstance();
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<NetworkClient> nc = n->GetClient(clientId);
    if (nc == nullptr)
        return;
    
    if (nc->player->tEntity == nullptr)
        return;

    std::string buildMode = nc->player->tEntity->BuildMode;
    
    if (mapId == -1)
        mapId = nc->player->tEntity->MapID;

    if (blockType == 1 && nc->player->tEntity->buildMaterial != -1)
        blockType = nc->player->tEntity->buildMaterial;
    
    if (_buildmodes.find(buildMode) == _buildmodes.end()) {
        nc->player->tEntity->BuildMode = "Normal";
        Logger::LogAdd(MODULE_NAME, "Could not find build mode '" + buildMode + "'.", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    std::shared_ptr<Map> playerMap = mm->GetPointer(mapId);

    if (_buildmodes[buildMode].Plugin == "") {
        playerMap->BlockChange(nc, X, Y, Z, mode, blockType);
    } else {
        BlockResend queuedItem{};
        queuedItem.X = X;
        queuedItem.Y = Y;
        queuedItem.Z = Z;
        queuedItem.clientId = clientId;
        queuedItem.mapId = mapId;
        _resendBlocks.insert(_resendBlocks.begin(), queuedItem);
        // -- Lua_Event_Build_Mode()
    }
}

void BuildModeMain::Resend(int clientId) {

}

void BuildModeMain::SetMode(int clientId, std::string mode) {

}
