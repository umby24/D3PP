#include "plugins/LuaPlugin.h"
#include <lua.hpp>

#include <filesystem>
#include <events/PlayerEventArgs.h>

#include "plugins/LuaState.h"

#include "world/Map.h"
#include "common/Logger.h"
#include "Utils.h"
#include "EventSystem.h"
#include "Block.h"
// -- Events..
#include "events/EventChatAll.h"
#include "events/EventChatMap.h"
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

// This template wraps a member function into a C-style "free" function compatible with lua.

void bail(lua_State* l, const std::string& errMsg) {
    Logger::LogAdd("LuaState", errMsg + ", Error: " + lua_tostring(l, -1), LogType::L_ERROR, GLF);
    lua_pop(l, 1);
}

LuaPlugin::~LuaPlugin() {
    lua_close(m_luaState->GetState());
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
    Dispatcher::subscribe(PlayerClickEventArgs{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
}

void LuaPlugin::HandleEvent(Event& event) {
    if (!event.PushLua)
        return;
    
    if (!m_loaded)
        return;

    auto type = event.type(); // -- get the type..

    std::shared_lock lock (m_luaState->eventMutex);
    if(m_luaState->events.find( type ) == m_luaState->events.end() ) // -- find all functions that want to be called on this event
        return;

    auto&& observers = m_luaState->events.at( type );

    for( auto&& observer : observers ) {
        executionMutex.lock();
        lua_getglobal(m_luaState->GetState(), observer.second.functionName.c_str()); // -- Get the function to be called
        if (!lua_isfunction(m_luaState->GetState(), -1)) {
            lua_pop(m_luaState->GetState(), 1);
            executionMutex.unlock();
            continue;
        }
        int argCount = event.PushLua(m_luaState->GetState()); // -- Have the event push its args and return how many were pushed..
        try {
            if (lua_pcall(m_luaState->GetState(), argCount, 1, 0) != 0) { // -- Call the function.
                bail(m_luaState->GetState(), "[Event Handler]"); // -- catch errors
                executionMutex.unlock();
                return;
            }
            int result = luaL_optinteger(m_luaState->GetState(), -1, 1);
            
            if (result == 0) {
                event.setCancelled();
            }

        } catch (const int exception) {
            bail(m_luaState->GetState(), "[Error Handler]"); // -- catch errors
            executionMutex.unlock();
            return;
        }
        executionMutex.unlock();
        // -- done.
    }
}

void LuaPlugin::Init() {
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
    if (!m_loaded)
        return;

    LoadNewOrChanged();

    std::unique_lock lock(m_luaState->eventMutex);
    for(auto &e : m_luaState->modifyList) {
        for (auto &i : m_luaState->events) {
            if (e.first == "del" && m_luaState->events[i.first].contains(e.second.eventId)) {
                m_luaState->events[i.first].erase(e.second.eventId);
            }
        }
        if (e.first == "add") {
            if (m_luaState->events.find(e.second.type) == m_luaState->events.end())
                m_luaState->events.insert(std::make_pair(e.second.type, std::map<std::string, LuaEvent>()));

            if (m_luaState->events[e.second.type].find(e.second.eventId) != m_luaState->events[e.second.type].end()) {
                m_luaState->events[e.second.type][e.second.eventId] = e.second;
            } else {
                m_luaState->events[e.second.type].insert(std::make_pair(e.second.eventId, e.second));
            }
        }
    }

    m_luaState->modifyList.clear();
}

void LuaPlugin::TimerMain() {
    auto timerDescriptor = Dispatcher::getDescriptor("Timer");
    {
        std::shared_lock lock(m_luaState->eventMutex);

        if (m_luaState->events.find(timerDescriptor) == m_luaState->events.end())
            return;

        auto &eventsAt = m_luaState->events[timerDescriptor];
        for (auto &e: eventsAt) {
            if (clock() >= (e.second.lastRun + e.second.duration)) {
                e.second.lastRun = clock();
                executionMutex.lock();
                lua_getglobal(m_luaState->GetState(),
                              e.second.functionName.c_str()); // -- Get the function to be called
                if (!lua_isfunction(m_luaState->GetState(), -1)) {
                    lua_pop(m_luaState->GetState(), 1);
                    executionMutex.unlock();
                    continue;
                }
                if (!lua_checkstack(m_luaState->GetState(), 1)) {

                }
                lua_pushinteger(m_luaState->GetState(), e.second.mapId);
                int result = lua_pcall(m_luaState->GetState(), 1, 0, 0);
                if (result == LUA_ERRRUN) {
                    bail(m_luaState->GetState(), "[Timer Event Handler]" + e.second.functionName); // -- catch errors
                    executionMutex.unlock();
                    break;
                } else if (result == LUA_ERRMEM) {
                    bail(m_luaState->GetState(), "[Timer Event Handler]" + e.second.functionName); // -- catch errors
                    executionMutex.unlock();
                    break;
                } else if (result == LUA_ERRERR) {
                    bail(m_luaState->GetState(), "[Timer Event Handler]" + e.second.functionName); // -- catch errors
                    executionMutex.unlock();
                    break;
                } else {
                    // -- success? maybe? who knows.

                }
                executionMutex.unlock();
            }
        }
    }
    // -- Make all modifications to the events list occur after events have been triggered.
    // -- This ensures no modification of in memory lists, and allows events to be created and destroyed from within other events.
}

void LuaPlugin::TriggerMapFill(int mapId, int sizeX, int sizeY, int sizeZ, const std::string& function, const std::string& args) {
    if (!m_loaded)
        return;
    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);
    lua_getglobal(m_luaState->GetState(), function.c_str());
    if (lua_isfunction(m_luaState->GetState(), -1)) {
        lua_pushinteger(m_luaState->GetState(), static_cast<lua_Integer>(mapId));
        lua_pushinteger(m_luaState->GetState(), static_cast<lua_Integer>(sizeX));
        lua_pushinteger(m_luaState->GetState(), static_cast<lua_Integer>(sizeY));
        lua_pushinteger(m_luaState->GetState(), static_cast<lua_Integer>(sizeZ));
        lua_pushstring(m_luaState->GetState(), args.c_str());
        if (lua_pcall(m_luaState->GetState(), 5, 0, 0)) {
            bail(m_luaState->GetState(), "Failed to run mapfill");
        }
    }
    else {
        lua_pop(m_luaState->GetState(), 1);
    }
}

void LuaPlugin::TriggerPhysics(int mapId, unsigned short X, unsigned short Y, unsigned short Z, const std::string& function) {
    if (!m_loaded)
        return;
    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);
    lua_getglobal(m_luaState->GetState(), function.c_str());
    if (lua_isfunction(m_luaState->GetState(), -1)) {
        lua_pushinteger(m_luaState->GetState(), static_cast<lua_Integer>(mapId));
        lua_pushinteger(m_luaState->GetState(), static_cast<lua_Integer>(X));
        lua_pushinteger(m_luaState->GetState(), static_cast<lua_Integer>(Y));
        lua_pushinteger(m_luaState->GetState(), static_cast<lua_Integer>(Z));
        if (lua_pcall(m_luaState->GetState(), 4, 0, 0) != 0) {
            bail(m_luaState->GetState(), "Failed to trig physics;");
        }
    }
    else {
        lua_pop(m_luaState->GetState(), 1);
    }
}

void LuaPlugin::TriggerCommand(const std::string& function, int clientId, const std::string& parsedCmd, const std::string& text0,
                               const std::string& text1, const std::string& op1, const std::string& op2, const std::string& op3, const std::string& op4,
                               const std::string& op5) {
    if (!m_loaded)
        return;
    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);
    lua_getglobal(m_luaState->GetState(), function.c_str());
    if (lua_isfunction(m_luaState->GetState(), -1)) {
        lua_pushinteger(m_luaState->GetState(), static_cast<lua_Integer>(clientId));
        lua_pushstring(m_luaState->GetState(), parsedCmd.c_str());
        lua_pushstring(m_luaState->GetState(), text0.c_str());
        lua_pushstring(m_luaState->GetState(), text1.c_str());
        lua_pushstring(m_luaState->GetState(), op1.c_str());
        lua_pushstring(m_luaState->GetState(), op2.c_str());
        lua_pushstring(m_luaState->GetState(), op3.c_str());
        lua_pushstring(m_luaState->GetState(), op4.c_str());
        lua_pushstring(m_luaState->GetState(), op5.c_str());
        if (lua_pcall(m_luaState->GetState(), 9, 0, 0)) {
            bail(m_luaState->GetState(), "Failed to run command.");
        }
    }
    else {
        lua_pop(m_luaState->GetState(), 1);
    }
}

void LuaPlugin::TriggerBuildMode(const std::string& function, int clientId, int mapId, unsigned short X, unsigned short Y,
                                 unsigned short Z, unsigned char mode, unsigned char block) {
    if (!m_loaded)
        return;
    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);
    lua_getglobal(m_luaState->GetState(), function.c_str());
    if (lua_isfunction(m_luaState->GetState(), -1)) {
        lua_pushinteger(m_luaState->GetState(), clientId);
        lua_pushinteger(m_luaState->GetState(), mapId);
        lua_pushinteger(m_luaState->GetState(), X);
        lua_pushinteger(m_luaState->GetState(), Y);
        lua_pushinteger(m_luaState->GetState(), Z);
        lua_pushinteger(m_luaState->GetState(), mode);
        lua_pushinteger(m_luaState->GetState(), block);
        if (lua_pcall(m_luaState->GetState(), 7, 0, 0)) {
            bail(m_luaState->GetState(), "Failed to run command.");
        }
    }
    else {
        lua_pop(m_luaState->GetState(), 1);
    }
}

void LuaPlugin::TriggerBlockCreate(const std::string& function, int mapId, unsigned short X, unsigned short Y, unsigned short Z)
{
    if (!m_loaded)
        return;
    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);
    lua_getglobal(m_luaState->GetState(), function.c_str());
    if (lua_isfunction(m_luaState->GetState(), -1)) {
        lua_pushinteger(m_luaState->GetState(), mapId);
        lua_pushinteger(m_luaState->GetState(), X);
        lua_pushinteger(m_luaState->GetState(), Y);
        lua_pushinteger(m_luaState->GetState(), Z);
        if (lua_pcall(m_luaState->GetState(), 4, 0, 0)) {
            bail(m_luaState->GetState(), "Failed to run Block Create.");
        }
    }
    else {
        lua_pop(m_luaState->GetState(), 1);
    }
}

void LuaPlugin::TriggerBlockDelete(const std::string& function, int mapId, unsigned short X, unsigned short Y, unsigned short Z)
{
    if (!m_loaded)
        return;

    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);
    lua_getglobal(m_luaState->GetState(), function.c_str());
    if (lua_isfunction(m_luaState->GetState(), -1)) {
        lua_pushinteger(m_luaState->GetState(), mapId);
        lua_pushinteger(m_luaState->GetState(), X);
        lua_pushinteger(m_luaState->GetState(), Y);
        lua_pushinteger(m_luaState->GetState(), Z);
        if (lua_pcall(m_luaState->GetState(), 4, 0, 0)) {
            bail(m_luaState->GetState(), "Failed to run Block Delete.");
        }
    }
    else {
        lua_pop(m_luaState->GetState(), 1);
    }
}

LuaPlugin::LuaPlugin(const std::string& folder) {
    m_folder = folder;
    m_luaState = std::make_shared<D3PP::plugins::LuaState>("Plugin " + folder);
    m_loaded = false;

    this->Interval = std::chrono::seconds(2);
    this->Main = [this] { MainFunc(); };

    TaskScheduler::RegisterTask("Plugin " + m_folder, *this);
}

void LuaPlugin::Load() {
    m_luaState->Create();
    m_luaState->RegisterApiLibs(m_luaState);
    RegisterEventListener();

    LoadNewOrChanged();
    if (_files.empty()) {
        Logger::LogAdd("LuaPlugin", "No lua files found, plugin is probably missing.", LogType::WARNING, GLF);
        return;
    }

    TaskItem newTi;
    newTi.Interval = std::chrono::milliseconds(10);
    newTi.Main = [this] { TimerMain(); };
    TaskScheduler::RegisterTask("LuaTimer", newTi);

    m_status = "Loaded";
    m_loaded = true;
}

void LuaPlugin::Unload()
{
    m_status = "Unloaded";
    m_loaded = false;
    m_luaState->Close();
}

std::string LuaPlugin::GetFolderName() {
    return m_folder;
}

bool LuaPlugin::IsLoaded() {
    return m_status == "Loaded";
}

void LuaPlugin::LoadNewOrChanged() {
    std::vector<std::string> pluginFiles = EnumerateDirectory(m_folder);

    for (auto const& file : pluginFiles) {
        auto i = _files.find(file);
        time_t lastMod = Utils::FileModTime(file);
        if (i != _files.end()) {
            // -- Note only accurate within +/- 1 second. shouldn't matter.
            if (i->second.LastLoaded != lastMod) {
                m_luaState->LoadFile(file, true);
                i->second.LastLoaded = lastMod;
            }

            continue;
        }

        LuaFile newFile;
        newFile.FilePath = file;
        newFile.LastLoaded = lastMod;
        _files.insert(std::make_pair(file, newFile));
        m_luaState->LoadFile(file, true);
    }
}

