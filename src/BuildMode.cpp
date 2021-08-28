#include <Block.h>
#include <plugins/LuaPlugin.h>
#include "BuildMode.h"
#include "Files.h"
#include "common/PreferenceLoader.h"
#include "Network.h"
#include "NetworkClient.h"
#include "Player.h"
#include "Entity.h"
#include "Map.h"
#include "Logger.h"
#include "Utils.h"
#include "Network_Functions.h"

BuildModeMain* BuildModeMain::Instance = nullptr;

const std::string MODULE_NAME = "BuildMode";

std::shared_ptr<NetworkClient> getClientById(int clientId) {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> nc = nm->GetClient(clientId);
    return nc;
}

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
        newBm.Plugin = pl.Read("Plugin", "");

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
        pl.Write("Plugin", item.second.Plugin);
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
        std::string pluginName = _buildmodes[buildMode].Plugin;
        Utils::replaceAll(pluginName, "Lua:", "");
        LuaPlugin* luaMain = LuaPlugin::GetInstance();
        luaMain->TriggerBuildMode(pluginName, clientId, mapId, X, Y, Z, mode, blockType);
        // -- Lua_Event_Build_Mode()
    }
}

void BuildModeMain::Resend(int clientId) {
    std::vector<int> toRemove;
    MapMain* mapMain= MapMain::GetInstance();

    for(auto i = 0; i< _resendBlocks.size(); i++) {
        if (_resendBlocks.at(i).clientId != clientId)
            continue;

        BlockResend &toResend = _resendBlocks.at(i);

        std::shared_ptr<Map> thisMap = mapMain->GetPointer(toResend.mapId);
        if (thisMap != nullptr) {
            int blockType = thisMap->GetBlockType(toResend.X, toResend.Y, toResend.Z);
            NetworkFunctions::NetworkOutBlockSet(clientId, toResend.X, toResend.Y, toResend.Z, blockType);
        }
        _resendBlocks.erase(_resendBlocks.begin() + i);
        if (i != 0)
            i--;
    }
}

void BuildModeMain::SetMode(int clientId, std::string mode) {
    std::shared_ptr<NetworkClient> c = getClientById(clientId);

    if (c->player && c->player->tEntity) {
        c->player->tEntity->BuildMode = mode;
        Resend(clientId);
    }
}



void BuildModeMain::SetState(int clientId, char state) {
    std::shared_ptr<NetworkClient> nc = getClientById(clientId);

    if (nc == nullptr)
        return;

    if (!nc->LoggedIn || nc->player == nullptr || nc->player->tEntity == nullptr)
        return;

    nc->player->tEntity->BuildState = state;
}

char BuildModeMain::GetState(int clientId) {
    char result = -1;

    std::shared_ptr<NetworkClient> nc = getClientById(clientId);
    if (nc == nullptr)
        return result;

    if (!nc->LoggedIn || nc->player == nullptr || nc->player->tEntity == nullptr)
        return result;

    result = nc->player->tEntity->BuildState;
    return result;
}

void BuildModeMain::SetCoordinate(int clientId, int index, unsigned short X, unsigned short Y, unsigned short Z) {
    std::shared_ptr<NetworkClient> nc = getClientById(clientId);

    if (nc == nullptr)
        return;

    if (!nc->LoggedIn || nc->player == nullptr || nc->player->tEntity == nullptr)
        return;

    if (index > Client_Player_Buildmode_Variables)
        return;

    nc->player->tEntity->variables[index].X = X;
    nc->player->tEntity->variables[index].Y = Y;
    nc->player->tEntity->variables[index].Z = Z;
}

unsigned short BuildModeMain::GetCoordinateX(int clientId, int index) {
    unsigned short result = 0;
    std::shared_ptr<NetworkClient> nc = getClientById(clientId);

    if (nc == nullptr)
        return result;

    if (!nc->LoggedIn || nc->player == nullptr || nc->player->tEntity == nullptr)
        return result;

    if (index > Client_Player_Buildmode_Variables)
        return result;

    result = nc->player->tEntity->variables[index].X;
    return result;
}

unsigned short BuildModeMain::GetCoordinateY(int clientId, int index) {
    unsigned short result = 0;
    std::shared_ptr<NetworkClient> nc = getClientById(clientId);

    if (nc == nullptr)
        return result;

    if (!nc->LoggedIn || nc->player == nullptr || nc->player->tEntity == nullptr)
        return result;

    if (index > Client_Player_Buildmode_Variables)
        return result;

    result = nc->player->tEntity->variables[index].Y;
    return result;
}

unsigned short BuildModeMain::GetCoordinateZ(int clientId, int index) {
    unsigned short result = 0;
    std::shared_ptr<NetworkClient> nc = getClientById(clientId);

    if (nc == nullptr)
        return result;

    if (!nc->LoggedIn || nc->player == nullptr || nc->player->tEntity == nullptr)
        return result;

    if (index > Client_Player_Buildmode_Variables)
        return result;

    result = nc->player->tEntity->variables[index].Z;
    return result;
}

void BuildModeMain::SetInt(int clientId, int index, int val) {
    std::shared_ptr<NetworkClient> nc = getClientById(clientId);

    if (nc == nullptr)
        return;

    if (!nc->LoggedIn || nc->player == nullptr || nc->player->tEntity == nullptr)
        return;

    if (index > Client_Player_Buildmode_Variables)
        return;

    nc->player->tEntity->variables[index].Long = val;
}

int BuildModeMain::GetInt(int clientId, int index) {
    int result = -1;
    std::shared_ptr<NetworkClient> nc = getClientById(clientId);

    if (nc == nullptr)
        return result;

    if (!nc->LoggedIn || nc->player == nullptr || nc->player->tEntity == nullptr)
        return result;

    if (index > Client_Player_Buildmode_Variables)
        return result;

    result = nc->player->tEntity->variables[index].Long;
    return result;
}

void BuildModeMain::SetFloat(int clientId, int index, float val) {
    std::shared_ptr<NetworkClient> nc = getClientById(clientId);

    if (nc == nullptr)
        return;

    if (!nc->LoggedIn || nc->player == nullptr || nc->player->tEntity == nullptr)
        return;

    if (index > Client_Player_Buildmode_Variables)
        return;

    nc->player->tEntity->variables[index].Float = val;
}

float BuildModeMain::GetFloat(int clientId, int index) {
    float result = -1;
    std::shared_ptr<NetworkClient> nc = getClientById(clientId);

    if (nc == nullptr)
        return result;

    if (!nc->LoggedIn || nc->player == nullptr || nc->player->tEntity == nullptr)
        return result;

    if (index > Client_Player_Buildmode_Variables)
        return result;

    result = nc->player->tEntity->variables[index].Float;

    return result;
}

void BuildModeMain::SetString(int clientId, int index, std::string val) {
    std::shared_ptr<NetworkClient> nc = getClientById(clientId);

    if (nc == nullptr)
        return;

    if (!nc->LoggedIn || nc->player == nullptr || nc->player->tEntity == nullptr)
        return;

    if (index > Client_Player_Buildmode_Variables)
        return;

    nc->player->tEntity->variables[index].String = val;
}

std::string BuildModeMain::GetString(int clientId, int index) {
    std::string result = "";
    std::shared_ptr<NetworkClient> nc = getClientById(clientId);

    if (nc == nullptr)
        return result;

    if (!nc->LoggedIn || nc->player == nullptr || nc->player->tEntity == nullptr)
        return result;

    if (index > Client_Player_Buildmode_Variables)
        return result;

    result = nc->player->tEntity->variables[index].String;
    return result;
}
