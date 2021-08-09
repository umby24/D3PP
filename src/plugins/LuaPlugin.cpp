#include "plugins/LuaPlugin.h"

#include <filesystem>

#include "Map.h"
#include "Network_Functions.h"
#include "Logger.h"
#include "Utils.h"
#include "Network.h"
#include "Player.h"
#include "Entity.h"
#include "BuildMode.h"
#include "Player_List.h"
#include "Build.h"
#include "Rank.h"

LuaPlugin* LuaPlugin::Instance = nullptr;

typedef int (LuaPlugin::*mem_func)(lua_State * L);

// This template wraps a member function into a C-style "free" function compatible with lua.
template <mem_func func>
int dispatch(lua_State * L) {
    LuaPlugin * ptr = *static_cast<LuaPlugin**>(lua_getextraspace(L));
    return ((*ptr).*func)(L);
}

void bail(lua_State *L, std::string msg) {
    Logger::LogAdd("Lua", "Fatal Error: " + msg + " " + lua_tostring(L, -1), LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
}


LuaPlugin::LuaPlugin() {
    this->Setup = [this] { Init(); };
    this->Main = [this] { MainFunc(); };
    this->Interval = std::chrono::seconds(1);

    TaskScheduler::RegisterTask("Lua", *this);

    state = luaL_newstate();
    luaL_openlibs(state);
    luaL_dostring(state, "print(\"hello world from lua\")");
    *static_cast<LuaPlugin**>(lua_getextraspace(this->state)) = this;
    BindFunctions();
}

LuaPlugin::~LuaPlugin() {
    lua_close(state);
}

LuaPlugin* LuaPlugin::GetInstance() {
    if (Instance == nullptr) {
        Instance = new LuaPlugin();
    }

    return Instance;
}

void LuaPlugin::BindFunctions() {
    // -- client functions:
    lua_register(state, "Client_Get_Table", &dispatch<&LuaPlugin::LuaClientGetTable>);
    lua_register(state, "Client_Get_Map_ID", &dispatch<&LuaPlugin::LuaClientGetMapId>);
    lua_register(state, "Client_Get_IP", &dispatch<&LuaPlugin::LuaClientGetIp>);
    lua_register(state, "Client_Get_Login_Name", &dispatch<&LuaPlugin::LuaClientGetLoginName>);
    lua_register(state, "Client_Get_Logged_In", &dispatch<&LuaPlugin::LuaClientGetLoggedIn>);
    lua_register(state, "Client_Get_Entity", &dispatch<&LuaPlugin::LuaClientGetEntity>);
    // -- Build Mode Functions
    lua_register(state, "Build_Mode_Set", &dispatch<&LuaPlugin::LuaBuildModeSet>);
    lua_register(state, "Build_Mode_Get", &dispatch<&LuaPlugin::LuaBuildModeGet>);
    lua_register(state, "Build_Mode_State_Set", &dispatch<&LuaPlugin::LuaBuildModeStateSet>);
    lua_register(state, "Build_Mode_State_Get", &dispatch<&LuaPlugin::LuaBuildModeStateGet>);
    lua_register(state, "Build_Mode_Coordinate_Set", &dispatch<&LuaPlugin::LuaBuildModeCoordinateSet>);
    lua_register(state, "Build_Mode_Coordinate_Get", &dispatch<&LuaPlugin::LuaBuildModeCoordinateGet>);
    lua_register(state, "Build_Mode_Long_Set", &dispatch<&LuaPlugin::LuaBuildModeLongSet>);
    lua_register(state, "Build_Mode_Long_Get", &dispatch<&LuaPlugin::LuaBuildModeLongGet>);
    lua_register(state, "Build_Mode_Float_Set", &dispatch<&LuaPlugin::LuaBuildModeFloatSet>);
    lua_register(state, "Build_Mode_Float_Get", &dispatch<&LuaPlugin::LuaBuildModeFloatGet>);
    lua_register(state, "Build_Mode_String_Set", &dispatch<&LuaPlugin::LuaBuildModeStringSet>);
    lua_register(state, "Build_Mode_String_Get", &dispatch<&LuaPlugin::LuaBuildModeStringGet>);
    // -- build functions
    lua_register(state, "Build_Line_Player", &dispatch<&LuaPlugin::LuaBuildLinePlayer>);
    lua_register(state, "Build_Box_Player", &dispatch<&LuaPlugin::LuaBuildBoxPlayer>);
    lua_register(state, "Build_Sphere_Player", &dispatch<&LuaPlugin::LuaBuildSpherePlayer>);
    lua_register(state, "Build_Rank_Box", &dispatch<&LuaPlugin::LuaBuildRankBox>);
    // -- Entity Functions:
    lua_register(state, "Entity_Get_Table", &dispatch<&LuaPlugin::LuaEntityGetTable>);
    lua_register(state, "Entity_Add", &dispatch<&LuaPlugin::LuaEntityAdd>);
    lua_register(state, "Entity_Delete", &dispatch<&LuaPlugin::LuaEntityDelete>);
    lua_register(state, "Entity_Get_Player", &dispatch<&LuaPlugin::LuaEntityGetPlayer>);
    lua_register(state, "Entity_Get_Map_ID", &dispatch<&LuaPlugin::LuaEntityGetMapId>);
    lua_register(state, "Entity_Get_X", &dispatch<&LuaPlugin::LuaEntityGetX>);
    lua_register(state, "Entity_Get_Y", &dispatch<&LuaPlugin::LuaEntityGetY>);
    lua_register(state, "Entity_Get_Z", &dispatch<&LuaPlugin::LuaEntityGetZ>);
    lua_register(state, "Entity_Get_Rotation", &dispatch<&LuaPlugin::LuaEntityGetRotation>);
    lua_register(state, "Entity_Get_Look", &dispatch<&LuaPlugin::LuaEntityGetLook>);
    lua_register(state, "Entity_Resend", &dispatch<&LuaPlugin::LuaEntityResend>);
    lua_register(state, "Entity_Message_2_Clients", &dispatch<&LuaPlugin::LuaEntityMessage2Clients>);
    lua_register(state, "Entity_Displayname_Get", &dispatch<&LuaPlugin::LuaEntityDisplaynameGet>);
    lua_register(state, "Entity_Displayname_Set", &dispatch<&LuaPlugin::LuaEntityDisplaynameSet>);
    lua_register(state, "Entity_Position_Set", &dispatch<&LuaPlugin::LuaEntityPositionSet>);
    lua_register(state, "Entity_Kill", &dispatch<&LuaPlugin::LuaEntityKill>);
    // -- Player functions:
    lua_register(state, "Player_Get_Table", &dispatch<&LuaPlugin::LuaPlayerGetTable>);
    lua_register(state, "Player_Get_Prefix", &dispatch<&LuaPlugin::LuaPlayerGetPrefix>);
    lua_register(state, "Player_Get_Name", &dispatch<&LuaPlugin::LuaPlayerGetName>);
    lua_register(state, "Player_Get_Suffix", &dispatch<&LuaPlugin::LuaPlayerGetSuffix>);
    lua_register(state, "Player_Get_IP", &dispatch<&LuaPlugin::LuaPlayerGetIp>);
    lua_register(state, "Player_Get_Rank", &dispatch<&LuaPlugin::LuaPlayerGetRank>);
    lua_register(state, "Player_Get_Online", &dispatch<&LuaPlugin::LuaPlayerGetOnline>);
    lua_register(state, "Player_Get_Ontime", &dispatch<&LuaPlugin::LuaPlayerGetOntime>);
    lua_register(state, "Player_Get_Mute_Time", &dispatch<&LuaPlugin::LuaPlayerGetMuteTime>);
    lua_register(state, "Player_Set_Rank", &dispatch<&LuaPlugin::LuaPlayerSetRank>);
    lua_register(state, "Player_Kick", &dispatch<&LuaPlugin::LuaPlayerKick>);
    lua_register(state, "Player_Ban", &dispatch<&LuaPlugin::LuaPlayerBan>);
    lua_register(state, "Player_Unban", &dispatch<&LuaPlugin::LuaPlayerUnban>);
    lua_register(state, "Player_Stop", &dispatch<&LuaPlugin::LuaPlayerStop>);
    lua_register(state, "Player_Unstop", &dispatch<&LuaPlugin::LuaPlayerUnstop>);
    lua_register(state, "Player_Mute", &dispatch<&LuaPlugin::LuaPlayerMute>);
    lua_register(state, "Player_Unmute", &dispatch<&LuaPlugin::LuaPlayerUnmute>);
    // -- Map Functions:
    lua_register(state, "Map_Block_Change", &dispatch<&LuaPlugin::LuaMapBlockChange>);
    lua_register(state, "Map_Block_Change_Client", &dispatch<&LuaPlugin::LuaMapBlockChangeClient>);
    lua_register(state, "Map_Block_Get_Type", &dispatch<&LuaPlugin::LuaMapBlockGetType>);
    lua_register(state, "Map_Block_Get_Player_Last", &dispatch<&LuaPlugin::LuaMapBlockGetPlayer>);
    // -- System Functions
    lua_register(state, "System_Message_Network_Send_2_All", &dispatch<&LuaPlugin::LuaMessageToAll>);
    lua_register(state, "System_Message_Network_Send", &dispatch<&LuaPlugin::LuaMessage>);
    lua_register(state, "Lang_Get", &dispatch<&LuaPlugin::LuaLanguageGet>);
}

void LuaPlugin::Init() {
    if (!std::filesystem::exists("Lua"))
        std::filesystem::create_directory("Lua");
}

std::vector<std::string> EnumerateDirectory(std::string dir) {
    std::vector<std::string> result;

    if (std::filesystem::is_directory(dir)) {
        for (const auto &entry : std::filesystem::directory_iterator(dir)) {
            std::string fileName = entry.path().filename().string();

            if (fileName.length() < 3)
                continue;

            if (entry.is_directory()) {
                std::vector<std::string> recurseResult = EnumerateDirectory(entry.path().string());
                result.insert(result.end(), recurseResult.begin(), recurseResult.end());
            }

            if (fileName.substr(fileName.length() - 3) == "lua") {
                result.push_back(entry.path().string());
            }
        }
    }

    return result;
}

void LuaPlugin::MainFunc() {
    std::vector<std::string> files = EnumerateDirectory("Lua/");

    for (auto const f : files) {
        auto i = _files.find(f);
        if (i != _files.end()) {
            time_t lastMod = Utils::FileModTime(f);
            // -- Note only accurate within +/- 1 second. shouldn't matter.
            if (i->second.LastLoaded != lastMod) {
                LoadFile(f);
                i->second.LastLoaded = lastMod;
            }

            continue;
        }
        LuaFile newFile;
        newFile.FilePath = f;
        _files.insert(std::make_pair(f, newFile));
    }
}

int LuaPlugin::LuaMapBlockChange(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 10) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Change called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    int mapId = lua_tointeger(L, 2);
    int X = lua_tointeger(L, 3);
    int Y = lua_tointeger(L, 4);
    int Z = lua_tointeger(L, 5);
    unsigned char type = lua_tointeger(L, 6);
    bool Undo = (lua_tointeger(L, 7) > 0);
    bool physics = (lua_tointeger(L, 8) > 0);
    bool send = (lua_tointeger(L, 9) > 0);
    unsigned char priority = lua_tointeger(L, 10);
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        map->BlockChange(playerNumber, X, Y, Z, type, Undo, physics, send, priority);
    }

    return 0;
}

int LuaPlugin::LuaMapBlockChangeClient(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 7) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Change_Client called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    int mapId = lua_tointeger(L, 2);
    int X = lua_tointeger(L, 3);
    int Y = lua_tointeger(L, 4);
    int Z = lua_tointeger(L, 5);
    unsigned char mode = lua_tointeger(L, 6);
    unsigned char type = lua_tointeger(L, 7);
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> client = nm->GetClient(clientId);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (client == nullptr || map== nullptr) {
        return 0;
    }

    map->BlockChange(client, X, Y, Z, mode, type);
    return 0;
}


int LuaPlugin::LuaMapBlockGetType(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Get_Type called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    int X = lua_tointeger(L, 2);
    int Y = lua_tointeger(L, 3);
    int Z = lua_tointeger(L, 4);

    int result = -1;
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        result = map->GetBlockType(X, Y, Z);
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaMessageToAll(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs < 2) {
        Logger::LogAdd("Lua", "LuaError: System_Message_Network_Send_2_All called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    std::string message = lua_tostring(L, 2);

    int messageType = 0;

    if (nArgs == 3 && lua_isnumber(L, 3)) {
        messageType = lua_tointeger(L, 3);
    }

    NetworkFunctions::SystemMessageNetworkSend2All(mapId, message, messageType);
    return 0;
}

int LuaPlugin::LuaMessage(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs < 2) {
        Logger::LogAdd("Lua", "LuaError: System_Message_Network_Send called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    std::string message = lua_tostring(L, 2);

    int messageType = 0;

    if (nArgs == 3 && lua_isnumber(L, 3)) {
        messageType = lua_tointeger(L, 3);
    }

    NetworkFunctions::SystemMessageNetworkSend(clientId, message, messageType);
    return 0;
}


void LuaPlugin::LoadFile(std::string path) {
    if(luaL_loadfile(this->state, path.c_str())) {
        bail(this->state, "Failed to load " + path);
        return;
    }

    if (lua_pcall(this->state, 0, 0, 0)) {
        bail(this->state, "Failed to load " + path);
    }
    // -- File loaded successfully.
}

void LuaPlugin::TriggerMapFill(int mapId, int sizeX, int sizeY, int sizeZ, std::string function, std::string args) {
    lua_getglobal(state, function.c_str());
    if (lua_isfunction(state, -1)) {
        lua_pushinteger(state, mapId);
        lua_pushinteger(state, sizeX);
        lua_pushinteger(state, sizeY);
        lua_pushinteger(state, sizeZ);
        lua_pushstring(state, args.c_str());
        if (lua_pcall(state, 5, 0, 0)) {
            bail(state, "Failed to run mapfill");
        }
    }
}

void LuaPlugin::TriggerPhysics(int mapId, unsigned short X, unsigned short Y, unsigned short Z, std::string function) {
    lua_getglobal(state, function.c_str());
    if (lua_isfunction(state, -1)) {
        lua_pushinteger(state, mapId);
        lua_pushinteger(state, X);
        lua_pushinteger(state, Y);
        lua_pushinteger(state, Z);
        lua_pcall(state, 4, 0, 0);
    }
}

void LuaPlugin::TriggerCommand(std::string function, int clientId, std::string parsedCmd, std::string text0,
                               std::string text1, std::string op1, std::string op2, std::string op3, std::string op4,
                               std::string op5) {

    lua_getglobal(state, function.c_str());
    if (lua_isfunction(state, -1)) {
        lua_pushinteger(state, clientId);
        lua_pushstring(state, parsedCmd.c_str());
        lua_pushstring(state, text0.c_str());
        lua_pushstring(state, text1.c_str());
        lua_pushstring(state, op1.c_str());
        lua_pushstring(state, op2.c_str());
        lua_pushstring(state, op3.c_str());
        lua_pushstring(state, op4.c_str());
        lua_pushstring(state, op5.c_str());
        if (lua_pcall(state, 9, 0, 0)) {
            bail(state, "Failed to run command.");
        }
    }
}

void LuaPlugin::TriggerBuildMode(std::string function, int clientId, int mapId, unsigned short X, unsigned short Y,
                                 unsigned short Z, unsigned char mode, unsigned char block) {
    lua_getglobal(state, function.c_str());
    if (lua_isfunction(state, -1)) {
        lua_pushinteger(state, clientId);
        lua_pushinteger(state, mapId);
        lua_pushinteger(state, X);
        lua_pushinteger(state, Y);
        lua_pushinteger(state, Z);
        lua_pushinteger(state, mode);
        lua_pushinteger(state, block);
        if (lua_pcall(state, 7, 0, 0)) {
            bail(state, "Failed to run command.");
        }
    }
}

int LuaPlugin::LuaMapBlockGetPlayer(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: Map_Block_Get_Player_Last called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int mapId = lua_tointeger(L, 1);
    int X = lua_tointeger(L, 2);
    int Y = lua_tointeger(L, 3);
    int Z = lua_tointeger(L, 4);

    int result = -1;
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map != nullptr) {
        result = map->GetBlockPlayer(X, Y, Z);
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaClientGetTable(lua_State *L) {
    Network* nm = Network::GetInstance();
    int numClients = nm->_clients.size();
    int index = 1;

    lua_newtable(L);

    if (numClients > 0) {
        for (auto const &nc : nm->_clients) {
            lua_pushinteger(L, index++);
            lua_pushinteger(L, nc.first);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numClients);

    return 2;
}

int LuaPlugin::LuaClientGetMapId(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: CLient_Get_Map_Id called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    int result = -1;
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> client = nm->GetClient(clientId);
    if (client != nullptr) {
        result = client->player->MapId;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaClientGetIp(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: CLient_Get_IP called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    std::string result = "";
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> client = nm->GetClient(clientId);
    if (client != nullptr) {
        result = client->IP;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaPlugin::LuaClientGetLoginName(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: CLient_Get_Login_Name called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    std::string result = "";
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> client = nm->GetClient(clientId);

    if (client != nullptr) {
        result = client->player->LoginName;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaPlugin::LuaClientGetLoggedIn(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: CLient_Get_Map_Id called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    int result = -1;

    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> client = nm->GetClient(clientId);

    if (client != nullptr) {
        result = client->LoggedIn;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaClientGetEntity(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: CLient_Get_Map_Id called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = lua_tointeger(L, 1);
    int result = -1;
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> client = nm->GetClient(clientId);

    if (client != nullptr) {
        if (client->player != nullptr && client->player->tEntity != nullptr)
            result = client->player->tEntity->Id;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaEntityGetTable(lua_State *L) {
    int numEntities = Entity::_entities.size();
    int index = 1;

    lua_newtable(L);

    if (numEntities > 0) {
        for (auto const &e : Entity::_entities) {
            lua_pushinteger(L, index++);
            lua_pushinteger(L, e.first);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numEntities);

    return 2;
}

int LuaPlugin::LuaEntityAdd(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 7) {
        Logger::LogAdd("Lua", "LuaError: Entity_Add called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    std::string eName(lua_tostring(L, 1));
    int mapId = lua_tointeger(L, 2);
    float x = lua_tonumber(L, 3);
    float y = lua_tonumber(L, 4);
    float z = lua_tonumber(L, 5);
    float rotation = lua_tonumber(L, 6);
    float look = lua_tonumber(L, 7);
    std::shared_ptr<Entity> newEntity = std::make_shared<Entity>(eName, mapId, x, y, z, rotation, look);
    Entity::Add(newEntity);
    int result = newEntity->Id;

    lua_pushinteger(L, result);

    return 1;
}

int LuaPlugin::LuaEntityDelete(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Entity_Delete called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    Entity::Delete(lua_tointeger(L, 1));

    return 0;
}

int LuaPlugin::LuaBuildModeSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    std::string buildMode(lua_tostring(L, 2));

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetMode(clientId, buildMode);

    return 0;
}

int LuaPlugin::LuaBuildModeGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> networkClient= nm->GetClient(clientId);

    if (networkClient != nullptr) {
        std::string clientBuildMode = networkClient->player->tEntity->BuildMode;
        lua_pushstring(L, clientBuildMode.c_str());
    } else {
        lua_pushstring(L, "");
    }

    return 1;
}

int LuaPlugin::LuaBuildModeStateSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_State_Set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int buildState = lua_tointeger(L, 2);

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetState(clientId, buildState);

    return 0;
}

int LuaPlugin::LuaBuildModeStateGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_State_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> networkClient= nm->GetClient(clientId);

    if (networkClient != nullptr) {
        lua_pushinteger(L, networkClient->player->tEntity->BuildState);
    } else {
        lua_pushinteger(L, -1);
    }

    return 1;
}

int LuaPlugin::LuaBuildModeCoordinateSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 5) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Coordinate_Set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    float X = lua_tonumber(L, 3);
    float Y = lua_tonumber(L, 4);
    float Z = lua_tonumber(L, 5);

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetCoordinate(clientId, index, X, Y, Z);

    return 0;
}

int LuaPlugin::LuaBuildModeCoordinateGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Coordinate_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    int X = -1;
    int Y = -1;
    int Z = -1;

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    X = buildModeMain->GetCoordinateX(clientId, index);
    Y = buildModeMain->GetCoordinateY(clientId, index);
    Z = buildModeMain->GetCoordinateZ(clientId, index);

    lua_pushinteger(L, X);
    lua_pushinteger(L, Y);
    lua_pushinteger(L, Z);

    return 3;
}

int LuaPlugin::LuaBuildModeLongSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Long_Set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    int X = lua_tonumber(L, 3);

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetInt(clientId, index, X);

    return 0;
}

int LuaPlugin::LuaBuildModeLongGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Long_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    int val = -1;

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    val = buildModeMain->GetInt(clientId, index);

    lua_pushinteger(L, val);
    return 1;
}

int LuaPlugin::LuaBuildModeFloatSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_float_Set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    float X = lua_tonumber(L, 3);

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetFloat(clientId, index, X);
    return 0;
}

int LuaPlugin::LuaBuildModeFloatGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_Float_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    float val = -1;

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    val = buildModeMain->GetFloat(clientId, index);

    lua_pushnumber(L, val);
    return 1;
}

int LuaPlugin::LuaBuildModeStringSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_String_Set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    std::string val(lua_tostring(L, 3));

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    buildModeMain->SetString(clientId, index, val);

    return 0;
}

int LuaPlugin::LuaBuildModeStringGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Build_Mode_String_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int clientId = lua_tointeger(L, 1);
    int index = lua_tointeger(L, 2);
    std::string val = "";

    BuildModeMain* buildModeMain = BuildModeMain::GetInstance();
    val = buildModeMain->GetString(clientId, index);

    lua_pushstring(L, val.c_str());
    return 1;
}

int LuaPlugin::LuaLanguageGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs < 2) {
        Logger::LogAdd("Lua", "LuaError: Lang_Get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    std::string language(lua_tostring(L, 1));
    std::string input(lua_tostring(L, 2));
//    if (n)
//    std::string Field0(lua_tostring(L, 3));
//    std::string Field1(lua_tostring(L, 4));
//    std::string Field2(lua_tostring(L, 5));
//    std::string Field3(lua_tostring(L, 6));

    lua_pushstring(L, input.c_str());
    return 1;
}

int LuaPlugin::LuaEntityGetPlayer(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_PLAYER called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    int result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->playerList->Number;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaEntityGetMapId(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_Map_Id called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    int result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->MapID;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaEntityGetX(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_X called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    float result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->X;
    }

    lua_pushnumber(L, result);
    return 1;
}

int LuaPlugin::LuaEntityGetY(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_Y called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    float result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->Y;
    }

    lua_pushnumber(L, result);
    return 1;
}

int LuaPlugin::LuaEntityGetZ(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_Z called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    float result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->Z;
    }

    lua_pushnumber(L, result);
    return 1;
}

int LuaPlugin::LuaEntityGetRotation(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_Rotation called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    float result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->Rotation;
    }

    lua_pushnumber(L, result);
    return 1;
}

int LuaPlugin::LuaEntityGetLook(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_Look called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    float result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->Look;
    }

    lua_pushnumber(L, result);
    return 1;
}

int LuaPlugin::LuaEntityResend(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Entity_Resend called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
        foundEntity->Resend(entityId);
    }

    return 0;
}

int LuaPlugin::LuaEntityMessage2Clients(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Entity_Message_2_Clients called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityid = lua_tointeger(L, 1);
    std::string message(lua_tostring(L, 2));

    Entity::MessageToClients(entityid, message);
    return 0;
}

int LuaPlugin::LuaEntityDisplaynameGet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_displayname_get called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
        std::string prefix(lua_tostring(L, 2));
        std::string displayName(lua_tostring(L, 3));
        std::string suffix(lua_tostring(L, 4));

        lua_pushstring(L, foundEntity->Prefix.c_str());
        lua_pushstring(L, foundEntity->Name.c_str());
        lua_pushstring(L, foundEntity->Suffix.c_str());
    }

    return 3;
}

int LuaPlugin::LuaEntityDisplaynameSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_displayname_set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    std::string prefix(lua_tostring(L, 2));
    std::string displayName(lua_tostring(L, 3));
    std::string suffix(lua_tostring(L, 4));

    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
        Entity::SetDisplayName(entityId, prefix, displayName, suffix);
    }

    return 0;
}

int LuaPlugin::LuaEntityPositionSet(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 7) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_Position_Set called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    int mapId = lua_tointeger(L, 2);
    float X = lua_tonumber(L, 3);
    float Y = lua_tonumber(L, 4);
    float Z = lua_tonumber(L, 5);
    float rotation = lua_tonumber(L, 6);
    float look = lua_tonumber(L, 7);

    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        foundEntity->PositionSet(mapId, X, Y, Z, rotation, look, 10, true);
    }

    return 0;
}

int LuaPlugin::LuaEntityKill(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_Kill called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int entityId = lua_tointeger(L, 1);
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
        foundEntity->Kill();
    }

    return 0;
}

int LuaPlugin::LuaBuildLinePlayer(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 12) {
        Logger::LogAdd("Lua", "LuaError: Build_Line_Player called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int playerNumber = lua_tointeger(L, 1);
    int mapId = lua_tointeger(L, 2);
    int x0 = lua_tointeger(L, 3);
    int y0 = lua_tointeger(L, 4);
    int z0 = lua_tointeger(L, 5);
    int x1 = lua_tointeger(L, 6);
    int y1 = lua_tointeger(L, 7);
    int z1 = lua_tointeger(L, 8);

    short material = lua_tointeger(L, 9);
    unsigned char priority = lua_tointeger(L, 10);
    bool undo = (lua_tointeger(L, 11) > 0);
    bool physics = lua_toboolean(L, 12);

    Build::BuildLinePlayer(playerNumber, mapId, x0, y0, z0, x1, y1, z1, material, priority, undo, physics);
    return 0;
}

int LuaPlugin::LuaBuildBoxPlayer(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 14) {
        Logger::LogAdd("Lua", "LuaError: Build_Box_Player called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    int mapId = lua_tointeger(L, 2);
    int x0 = lua_tointeger(L, 3);
    int y0 = lua_tointeger(L, 4);
    int z0 = lua_tointeger(L, 5);
    int x1 = lua_tointeger(L, 6);
    int y1 = lua_tointeger(L, 7);
    int z1 = lua_tointeger(L, 8);

    short material = lua_tointeger(L, 9);
    short replaceMaterial = lua_tointeger(L, 10);
    bool hollow = (lua_tointeger(L, 11) > 0);
    unsigned char priority = lua_tointeger(L, 12);
    bool undo = (lua_tointeger(L, 13) > 0);
    bool physics = lua_toboolean(L, 14);

    Build::BuildBoxPlayer(playerNumber, mapId, x0, y0, z0, x1, y1, z1, material, replaceMaterial, hollow, priority, undo, physics);
    return 0;
}

int LuaPlugin::LuaBuildSpherePlayer(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 12) {
        Logger::LogAdd("Lua", "LuaError: Build_Sphere_Player called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int playerNumber = lua_tointeger(L, 1);
    int mapId = lua_tointeger(L, 2);
    int x = lua_tointeger(L, 3);
    int y = lua_tointeger(L, 4);
    int z = lua_tointeger(L, 5);
    float radius = lua_tonumber(L, 6);
    short material = lua_tointeger(L, 7);
    short replaceMaterial = lua_tointeger(L, 8);
    bool hollow = (lua_tointeger(L, 9) > 0);
    unsigned char priority = lua_tointeger(L, 10);
    bool undo = (lua_tointeger(L, 11) > 0);
    bool physics = lua_toboolean(L, 12);

    Build::BuildSpherePlayer(playerNumber, mapId, x, y, z, radius, material, replaceMaterial, hollow, priority, undo, physics);

    return 0;
}

int LuaPlugin::LuaBuildRankBox(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 9) {
        Logger::LogAdd("Lua", "LuaError: Build_Rank_Box called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }
    int mapId = lua_tointeger(L, 1);
    int X0 = lua_tointeger(L, 2);
    int Y0 = lua_tointeger(L, 3);
    int Z0 = lua_tointeger(L, 4);
    int X1 = lua_tointeger(L, 5);
    int Y1 = lua_tointeger(L, 6);
    int Z1 = lua_tointeger(L, 7);
    int Rank = lua_tointeger(L, 8);
    int MaxRank = lua_tointeger(L, 9);

    Build::BuildRankBox(mapId, X0, Y0, Z0, X1, Y1, Z1, Rank, MaxRank);

    return 0;
}

int LuaPlugin::LuaPlayerGetTable(lua_State *L) {
    Player_List* pll = Player_List::GetInstance();
    int numEntities = pll->_pList.size();
    int index = 1;

    lua_newtable(L);

    if (numEntities > 0) {
        for (auto const &e :  pll->_pList) {
            lua_pushinteger(L, index++);
            lua_pushinteger(L, e.Number);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numEntities);

    return 2;
}

int LuaPlugin::LuaPlayerGetPrefix(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Prefix called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();
    Rank* rm = Rank::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    std::string result = "";

    if (ple != nullptr) {
        RankItem ri = rm->GetRank(ple->PRank, false);
        result = ri.Prefix;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaPlugin::LuaPlayerGetName(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Name called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    std::string result = "";

    if (ple != nullptr) {

        result = ple->Name;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaPlugin::LuaPlayerGetSuffix(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Suffix called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();
    Rank* rm = Rank::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    std::string result = "";

    if (ple != nullptr) {
        RankItem ri = rm->GetRank(ple->PRank, false);
        result = ri.Suffix;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}
int LuaPlugin::LuaPlayerGetIp(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Ip called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    std::string result = "";

    if (ple != nullptr) {
        result = ple->IP;
    }

    lua_pushstring(L, result.c_str());
    return 1;
}

int LuaPlugin::LuaPlayerGetOntime(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Ontime called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    int result = -1;

    if (ple != nullptr) {
        result = ple->OntimeCounter;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaPlayerGetRank(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Rank called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    int result = -1;

    if (ple != nullptr) {
        result = ple->PRank;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaPlayerGetOnline(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Online called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    int result = 0;

    if (ple != nullptr) {
        result = ple->Online;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaPlayerGetMuteTime(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Get_Mute_Time called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    Player_List* pll = Player_List::GetInstance();

    PlayerListEntry* ple = pll->GetPointer(playerNumber);
    int result = -1;

    if (ple != nullptr) {
        result = ple->MuteTime;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaPlugin::LuaPlayerSetRank(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Player_Set_Rank called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    int rankNumber = lua_tointeger(L, 2);
    std::string reason(lua_tostring(L, 3));

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->SetRank(rankNumber, reason);
    }

    return 0;
}

int LuaPlugin::LuaPlayerKick(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs > 2) {
        Logger::LogAdd("Lua", "LuaError: Player_Kick called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    std::string reason(lua_tostring(L, 2));
    int count = 0;
    bool log = true;
    bool show = true;

    if (nArgs >= 3)
        count = lua_tonumber(L, 3);

    if (nArgs >= 4)
        log = (lua_tonumber(L, 4) > 0);

    if (nArgs >= 5)
        show = (lua_tonumber(L, 5) > 0);

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Kick(reason, count, log, show);
    }

    return 0;
}

int LuaPlugin::LuaPlayerBan(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Player_Ban called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    std::string reason(lua_tostring(L, 2));

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Ban(reason);
    }

    return 0;
}

int LuaPlugin::LuaPlayerUnban(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Unban called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Unban();
    }

    return 0;
}

int LuaPlugin::LuaPlayerStop(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Player_Stop called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    std::string reason(lua_tostring(L, 2));

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Stop(reason);
    }

    return 0;
}

int LuaPlugin::LuaPlayerUnstop(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Unstop called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Unstop();
    }

    return 0;
}

int LuaPlugin::LuaPlayerMute(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: Player_Mute called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);
    int minutes = lua_tointeger(L, 2);
    std::string reason(lua_tostring(L, 3));

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Mute(minutes, reason);
    }

    return 0;
}

int LuaPlugin::LuaPlayerUnmute(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Player_Unmute called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int playerNumber = lua_tointeger(L, 1);

    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple = pll->GetPointer(playerNumber);

    if (ple != nullptr) {
        ple->Unmute();
    }

    return 0;
}









