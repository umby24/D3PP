#include "plugins/LuaPlugin.h"

#include <filesystem>

#include "Map.h"
#include "Network_Functions.h"
#include "Logger.h"
#include "Utils.h"

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
    lua_register(state, "Map_Block_Change", &dispatch<&LuaPlugin::LuaMapBlockChange>);
    lua_register(state, "Map_Block_Get_Type", &dispatch<&LuaPlugin::LuaMapBlockGetType>);
    lua_register(state, "System_Message_Network_Send_2_All", &dispatch<&LuaPlugin::LuaMessageToAll>);
}

void LuaPlugin::Init() {
    if (!std::filesystem::exists("Lua"))
        std::filesystem::create_directory("Lua");
}

void LuaPlugin::MainFunc() {
    std::vector<std::string> files;
    for (const auto & entry : std::filesystem::directory_iterator("Lua/")) {
        std::string fileName = entry.path().filename().string();
        if (fileName.length() < 3)
            continue;

        if (fileName.substr(fileName.length() - 3) == "lua") {
            files.push_back(entry.path().string());
        }
    }

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
    std::string message = lua_tostring(L, mapId);

    int messageType = 0;

    if (nArgs == 3 && lua_isnumber(L, 3)) {
        messageType = lua_tointeger(L, 3);
    }

    NetworkFunctions::SystemMessageNetworkSend2All(mapId, message, messageType);
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
        lua_pcall(state, 5, 0, 0);
    }
}
