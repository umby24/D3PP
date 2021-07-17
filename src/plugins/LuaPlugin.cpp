#include "plugins/LuaPlugin.h"

#include <filesystem>

#include "Map.h"
#include "Network_Functions.h"
#include "Logger.h"
#include "Utils.h"
#include "Network.h"
#include "Player.h"
#include "Entity.h"

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
    // -- Entity Functions:
    lua_register(state, "Entity_Get_Table", &dispatch<&LuaPlugin::LuaEntityGetTable>);
    lua_register(state, "Entity_Add", &dispatch<&LuaPlugin::LuaEntityAdd>);
    lua_register(state, "Entity_Delete", &dispatch<&LuaPlugin::LuaEntityDelete>);
    // -- Map Functions:
    lua_register(state, "Map_Block_Change", &dispatch<&LuaPlugin::LuaMapBlockChange>);
    lua_register(state, "Map_Block_Get_Type", &dispatch<&LuaPlugin::LuaMapBlockGetType>);
    lua_register(state, "Map_Block_Get_Player_Last", &dispatch<&LuaPlugin::LuaMapBlockGetPlayer>);
    // -- System Functions
    lua_register(state, "System_Message_Network_Send_2_All", &dispatch<&LuaPlugin::LuaMessageToAll>);
    lua_register(state, "System_Message_Network_Send", &dispatch<&LuaPlugin::LuaMessage>);
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




