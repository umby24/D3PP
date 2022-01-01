#include "plugins/LuaPlugin.h"

#include <filesystem>
#include <events/PlayerEventArgs.h>
#include <CustomBlocks.h>

#include "world/Map.h"
#include "network/Network_Functions.h"
#include "common/Logger.h"
#include "Utils.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "world/Player.h"
#include "world/Entity.h"
#include "common/Player_List.h"
#include "Rank.h"
#include "EventSystem.h"
#include "Block.h"
#include "common/MinecraftLocation.h"
#include "CPE.h"
#include "common/Files.h"

// -- Events..
#include "events/EventChatAll.h"
#include "events/EventChatMap.h"
#include "events/EventChatPrivate.h"
#include "events/EventClientAdd.h"
#include "events/EventClientDelete.h"
#include "events/EventClientLogin.h"
#include "events/EventClientLogout.h"
#include "events/EventEntityAdd.h"
#include "events/EventEntityDelete.h"
#include "events/EventEntityDie.h"
#include "events/EventEntityMapChange.h"
#include "events/EventEntityPositionSet.h"
#include "events/EventMapActionDelete.h"
#include "events/EventMapActionFill.h"
#include "events/EventMapActionLoad.h"
#include "events/EventMapActionResize.h"
#include "events/EventMapActionSave.h"
#include "events/EventMapAdd.h"
#include "events/EventMapBlockChange.h"
#include "events/EventMapBlockChangeClient.h"
#include "events/EventMapBlockChangePlayer.h"
#include "events/EventTimer.h"

#include "lua/block.h"
#include "lua/client.h"
#include "lua/buildmode.h"
#include "lua/build.h"
#include "lua/entity.h"
#include "lua/player.h"
#include "lua/map.h"
#include "lua/cpe.h"
#include "lua/rank.h"
#include "lua/system.h"
#include "lua/teleporter.h"
#include "lua/network.h"

LuaPlugin* LuaPlugin::Instance = nullptr;

typedef int (LuaPlugin::*mem_func)(lua_State * L);

// This template wraps a member function into a C-style "free" function compatible with lua.
template <mem_func func>
int dispatch(lua_State * L) {
    LuaPlugin * ptr = *static_cast<LuaPlugin**>(lua_getextraspace(L));
    return ((*ptr).*func)(L);
}

void bail(lua_State *L, const std::string& msg) {
    Logger::LogAdd("Lua", "Fatal Error: " + msg + " " + lua_tostring(L, -1), LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
}

static int l_add_block(lua_State *L)
{
    int nArgs = lua_gettop(L);
    
    if (nArgs != 2)
    {
        Logger::LogAdd("Lua", "Invalid num args", LogType::L_ERROR, GLF);
        return 0;
    }
    std::string blockName(luaL_checkstring(L, 1));
    int clientId = luaL_checkinteger(L, 2);

    MapBlock newBlock{ 127, blockName, clientId, 0, "", 0, 0, false, false, "", "", 0, 0, 0, 0, false, false, 0, 0, 0 };
    Block* bm = Block::GetInstance();
    bm->Blocks[127] = newBlock;

    return 0;
}

LuaPlugin::LuaPlugin() {
    this->Setup = [this] { Init(); };
    this->Main = [this] { MainFunc(); };
    this->Interval = std::chrono::seconds(1);

    TaskScheduler::RegisterTask("Lua", *this);
    // -- Custom D3 Libraries :) 
    LuaClientLib cLib;
    LuaBuildModeLib bmLib;
    LuaBuildLib bLib;
    LuaEntityLib eLib;
    LuaPlayerLib pLib;
    LuaMapLib mLib;
    LuaCPELib cpeLib;
    LuaRankLib rLib;
    LuaSystemLib sLib;
    LuaNetworkLib nLib;
    LuaBlockLib blLib;
    LuaTeleporterLib tpLib;

    state = luaL_newstate();
    luaL_openlibs(state);
    cLib.openLib(state);
    bmLib.openLib(state);
    bLib.openLib(state);
    eLib.openLib(state);
    pLib.openLib(state);
    mLib.openLib(state);
    cpeLib.openLib(state);
    rLib.openLib(state);
    sLib.openLib(state);
    nLib.openLib(state);
    blLib.openLib(state);
    tpLib.openLib(state);

    *static_cast<LuaPlugin**>(lua_getextraspace(this->state)) = this;

    BindFunctions();
    RegisterEventListener();

    TaskItem newTi;
    newTi.Interval = std::chrono::milliseconds(10);
    newTi.Main = [this] { TimerMain(); };
    TaskScheduler::RegisterTask("LuaTimer", newTi);
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

void LuaPlugin::RegisterEventListener() {
    Dispatcher::subscribe(EventChatAll{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventChatMap{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
   // Dispatcher::subscribe(EventChatPrivate{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventClientAdd{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventClientDelete{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventClientLogin{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventClientLogout{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventEntityAdd{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventEntityDelete{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventEntityDie{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventEntityMapChange{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventEntityPositionSet{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapActionDelete{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapActionFill{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapActionLoad{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapActionResize{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapActionSave{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapAdd{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapBlockChange{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapBlockChangeClient{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventMapBlockChangePlayer{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventTimer{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(PlayerClickEventArgs::clickDescriptor, [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
}

void LuaPlugin::HandleEvent(const Event& event) {
    if (!event.PushLua)
        return;

    auto type = event.type(); // -- get the type..

    if(events.find( type ) == events.end() ) // -- find all functions that want to be called on this event
        return;

    auto&& observers = events.at( type );

    for( auto&& observer : observers ) {
        executionMutex.lock();
        lua_getglobal(state, observer.functionName.c_str()); // -- Get the function to be called
        if (!lua_isfunction(state, -1)) {
            lua_pop(state, 1);
            executionMutex.unlock();
            continue;
        }
        int argCount = event.PushLua(state); // -- Have the event push its args and return how many were pushed..
        try {
            if (lua_pcall(state, argCount, 0, 0) != 0) { // -- Call the function.
                bail(state, "[Event Handler]"); // -- catch errors
                executionMutex.unlock();
                return;
            }
        } catch (const int exception) {
            bail(state, "[Error Handler]"); // -- catch errors
            executionMutex.unlock();
            return;
        }
        executionMutex.unlock();
        // -- done.
    }
}

void LuaPlugin::BindFunctions() {
    // -- Network functions:
}

void LuaPlugin::Init() {
    if (!std::filesystem::exists("Lua"))
        std::filesystem::create_directory("Lua");
}

std::vector<std::string> EnumerateDirectory(const std::string &dir) {
    std::vector<std::string> result{};

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

    for (auto const &f : files) {
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
        newFile.LastLoaded = 0;
        _files.insert(std::make_pair(f, newFile));
    }
  
}

void LuaPlugin::TimerMain() {
    auto timerDescriptor = Dispatcher::getDescriptor("Timer");
    if (events.find(timerDescriptor) == events.end())
        return;

    auto &eventsAt = events[timerDescriptor];
    for(auto &e : eventsAt) {
        if (clock() >= (e.lastRun + e.duration)) {
            e.lastRun = clock();
            executionMutex.lock();
            lua_getglobal(state, e.functionName.c_str()); // -- Get the function to be called
            if (!lua_isfunction(state, -1)) {
                lua_pop(state, 1);
                executionMutex.unlock();
                continue;
            }
            if (lua_pcall(state, 0, 0, 0) != 0) { // -- Call the function.
                bail(state, "[Timer Event Handler]"); // -- catch errors
                executionMutex.unlock();
                return;
            }
            executionMutex.unlock();
        }
    }
}

void LuaPlugin::LoadFile(const std::string& path) {
    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);

    if(luaL_loadfile(this->state, path.c_str())) {
        bail(this->state, "Failed to load " + path);
        return;
    }

    if (lua_pcall(this->state, 0, 0, 0)) {
        bail(this->state, "Failed to load " + path);
    }
    // -- File loaded successfully.
}

void LuaPlugin::TriggerMapFill(int mapId, int sizeX, int sizeY, int sizeZ, const std::string& function, const std::string& args) {
    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);
    lua_getglobal(state, function.c_str());
    if (lua_isfunction(state, -1)) {
        lua_pushinteger(state, static_cast<lua_Integer>(mapId));
        lua_pushinteger(state, static_cast<lua_Integer>(sizeX));
        lua_pushinteger(state, static_cast<lua_Integer>(sizeY));
        lua_pushinteger(state, static_cast<lua_Integer>(sizeZ));
        lua_pushstring(state, args.c_str());
        if (lua_pcall(state, 5, 0, 0)) {
            bail(state, "Failed to run mapfill");
        }
    }
}

void LuaPlugin::TriggerPhysics(int mapId, unsigned short X, unsigned short Y, unsigned short Z, const std::string& function) {
    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);
    lua_getglobal(state, function.c_str());
    if (lua_isfunction(state, -1)) {
        lua_pushinteger(state, static_cast<lua_Integer>(mapId));
        lua_pushinteger(state, static_cast<lua_Integer>(X));
        lua_pushinteger(state, static_cast<lua_Integer>(Y));
        lua_pushinteger(state, static_cast<lua_Integer>(Z));
        if (lua_pcall(state, 4, 0, 0) != 0) {
            bail(state, "Failed to trig physics;");
        }
    }
}

void LuaPlugin::TriggerCommand(const std::string& function, int clientId, const std::string& parsedCmd, const std::string& text0,
                               const std::string& text1, const std::string& op1, const std::string& op2, const std::string& op3, const std::string& op4,
                               const std::string& op5) {
    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);
    lua_getglobal(state, function.c_str());
    if (lua_isfunction(state, -1)) {
        lua_pushinteger(state, static_cast<lua_Integer>(clientId));
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

void LuaPlugin::TriggerBuildMode(const std::string& function, int clientId, int mapId, unsigned short X, unsigned short Y,
                                 unsigned short Z, unsigned char mode, unsigned char block) {
    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);
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


