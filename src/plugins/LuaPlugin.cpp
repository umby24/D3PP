#include "plugins/LuaPlugin.h"
#include <lua.hpp>

#include <filesystem>
#include <ranges>
#include "events/PlayerEventArgs.h"

#include "plugins/LuaState.h"

#include "common/Logger.h"
#include "Utils.h"
#include "EventSystem.h"
#include "Block.h"


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
    Logger::LogAdd("LuaState", errMsg + ", Error: " + lua_tostring(l, -1), L_ERROR, GLF);
    lua_pop(l, 1);
}

LuaPlugin::~LuaPlugin() {
    lua_close(m_luaState->GetState());
}

void LuaPlugin::RegisterEventListener() {
    Dispatcher::subscribe(EventChatAll{}.type(),       [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventChatMap{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
   // Dispatcher::subscribe(EventChatPrivate{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    Dispatcher::subscribe(EventClientAdd{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventClientDelete{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventClientLogin{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventClientLogout{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventEntityAdd{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventEntityDelete{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventEntityDie{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventEntityMapChange{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventEntityPositionSet{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventMapActionDelete{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventMapActionFill{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventMapActionLoad{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventMapActionResize{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventMapActionSave{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventMapAdd{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventMapBlockChange{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventMapBlockChangeClient{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventMapBlockChangePlayer{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(EventTimer{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
    Dispatcher::subscribe(PlayerClickEventArgs{}.type(), [this]<typename T0>(T0 && PH1) { HandleEvent(std::forward<T0>(PH1)); });
}

void LuaPlugin::HandleEvent(Event& event) {
    if (event.PushLua == nullptr)
        return;
    
    if (!m_loaded)
        return;

    const auto type = event.type(); // -- get the type..

    std::shared_lock lock (m_luaState->eventMutex); // -- Deadlock Source
    if(!m_luaState->events.contains( type )) // -- find all functions that want to be called on this event
        return;

    for(auto &&observers = m_luaState->events.at(type); auto &val: observers | std::views::values) {
        std::scoped_lock rcLock(executionMutex);
        lua_State* thisLuaState = m_luaState->GetState();

        lua_getglobal(thisLuaState, val.functionName.c_str()); // -- Get the function to be called

        if (!lua_isfunction(thisLuaState, -1)) {
            lua_pop(thisLuaState, 1);
            continue;
        }

        const int argCount = event.PushLua(thisLuaState); // -- Have the event push its args and return how many were pushed..
        try {
            if (lua_pcall(thisLuaState, argCount, 1, 0) != 0) { // -- Call the function.
                bail(thisLuaState, "[Event Handler]"); // -- catch errors
                return;
            }
            if (lua_isinteger(thisLuaState, -1)) {
                if (const int result = static_cast<int>(lua_tointeger(thisLuaState, -1)); result == 0) {
                    event.setCancelled();
                }
            }

            lua_pop(thisLuaState, 1);
        } catch (const int exception) {
            bail(thisLuaState, "[Error Handler]"); // -- catch errors
            return;
        }
        // -- done.
    }
}

void LuaPlugin::Init() {
}

/**
 * Recursively traverse a directory and collect a list of all lua files within that path.
 * @param dir The starting directory to traverse
 * @return Paths (relative to dir) to lua files.
 */
std::vector<std::string> EnumerateDirectory(const std::string &dir) { // NOLINT(*-no-recursion)
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

    // -- Load any new or modified lua files.
    LoadNewOrChanged();

    // -- Prune our events listing. Requires unique lock of event mutex.
    {
        std::unique_lock lock(m_luaState->eventMutex); // -- Deadlock something..
        for (auto& e : m_luaState->modifyList) {
            if (e.first.starts_with("del")) { // -- Remove items earmarked for removal.
                for (auto& i : m_luaState->events) {
                    if (m_luaState->events[i.first].contains(e.second.eventId)) {
                        m_luaState->events[i.first].erase(e.second.eventId);
                    }
                }
            }
            if (e.first.starts_with("add")) { // -- Register new events.
                if (!m_luaState->events.contains(e.second.type)) // -- Does this type exist in our tracker yet? i.e. EVENT_ENTITY_ADD.
                    m_luaState->events.insert(std::make_pair(e.second.type, std::map<std::string, LuaEvent>())); // -- If not create a base list for it.

                if (m_luaState->events[e.second.type].contains(e.second.eventId)) { // -- Does this event list have our event id in it?
                    m_luaState->events[e.second.type][e.second.eventId] = e.second; // -- if yes lets just reassign it.
                }
                else { // -- Otherwise we need to insert it.
                    m_luaState->events[e.second.type].insert(std::make_pair(e.second.eventId, e.second));
                }
            }
        }

        m_luaState->modifyList.clear(); // -- Clear our earmark list.
    }
}

void LuaPlugin::TimerMain() { {
        const auto timerDescriptor = Dispatcher::getDescriptor("Timer");
        std::shared_lock lock(m_luaState->eventMutex);
        if (!m_luaState->events.contains(timerDescriptor))
            return;

        for (auto &eventsAt = m_luaState->events[timerDescriptor]; auto &val: eventsAt | std::views::values) {
            if (clock() >= (val.lastRun + val.duration)) {
                val.lastRun = clock();
                std::scoped_lock<std::recursive_mutex> rcLock(executionMutex);
                lua_getglobal(m_luaState->GetState(),
                              val.functionName.c_str()); // -- Get the function to be called
                if (!lua_isfunction(m_luaState->GetState(), -1)) {
                    lua_pop(m_luaState->GetState(), 1);
                    continue;
                }
                if (!lua_checkstack(m_luaState->GetState(), 1)) {

                }
                lua_pushinteger(m_luaState->GetState(), val.mapId);
                int result = lua_pcall(m_luaState->GetState(), 1, 0, 0);
                if (result == LUA_ERRRUN) {
                    bail(m_luaState->GetState(), "[Timer Event Handler]" + val.functionName); // -- catch errors
                    break;
                } else if (result == LUA_ERRMEM) {
                    bail(m_luaState->GetState(), "[Timer Event Handler]" + val.functionName); // -- catch errors
                    break;
                } else if (result == LUA_ERRERR) {
                    bail(m_luaState->GetState(), "[Timer Event Handler]" + val.functionName); // -- catch errors
                    break;
                } else {
                    // -- success? maybe? who knows.
                    lua_pop(m_luaState->GetState(), lua_gettop(m_luaState->GetState()));
                }
            }
        }
    }
    // -- Make all modifications to the events list occur after events have been triggered.
    // -- This ensures no modification of in memory lists, and allows events to be created and destroyed from within other events.
}

void LuaPlugin::TriggerMapFill(int mapId, int sizeX, int sizeY, int sizeZ, const std::string& function, const std::string& args) {
    if (!m_loaded)
        return;
    std::scoped_lock pqlock(executionMutex);
    lua_getglobal(m_luaState->GetState(), function.c_str());
    if (lua_isfunction(m_luaState->GetState(), -1)) {
        lua_pushinteger(m_luaState->GetState(), mapId);
        lua_pushinteger(m_luaState->GetState(), sizeX);
        lua_pushinteger(m_luaState->GetState(), sizeY);
        lua_pushinteger(m_luaState->GetState(), sizeZ);
        lua_pushstring(m_luaState->GetState(), args.c_str());
        if (lua_pcall(m_luaState->GetState(), 5, 0, 0)) {
            bail(m_luaState->GetState(), "Failed to run mapfill");
        } else {
            lua_pop(m_luaState->GetState(), lua_gettop(m_luaState->GetState()));
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
        } else {
            lua_pop(m_luaState->GetState(), lua_gettop(m_luaState->GetState()));
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
        } else {
            lua_pop(m_luaState->GetState(), lua_gettop(m_luaState->GetState()));
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
        } else {
            lua_pop(m_luaState->GetState(), lua_gettop(m_luaState->GetState()));
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
        } else {

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
        } else {
            lua_pop(m_luaState->GetState(), lua_gettop(m_luaState->GetState()));
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
    this->LastRun = std::chrono::system_clock::now();

    TaskScheduler::RegisterTask("Plugin " + m_folder, *this);
}

void LuaPlugin::Load() {
    m_luaState->Create();
    m_luaState->RegisterApiLibs(m_luaState);
    RegisterEventListener();

    LoadNewOrChanged();
    if (_files.empty()) {
        Logger::LogAdd("LuaPlugin", "No lua files found, plugin is probably missing.", WARNING, GLF);
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

/**
 * Iterate a plugin directory and load new files or reload modified files.
 */
void LuaPlugin::LoadNewOrChanged() {
    std::vector<std::string> pluginFiles = EnumerateDirectory(m_folder);

    for (auto const& file : pluginFiles) {
        auto i = _files.find(file);
        time_t lastMod = Utils::FileModTime(file);
        if (i != _files.end()) {
            // -- Note only accurate within +/- 1 second. shouldn't matter.
            if (i->second.LastLoaded != lastMod) {
                std::scoped_lock<std::recursive_mutex> rcLock(executionMutex);
                m_luaState->LoadFile(file, true);
                i->second.LastLoaded = lastMod;
            }

            continue;
        }

        LuaFile newFile;
        newFile.FilePath = file;
        newFile.LastLoaded = lastMod;
        std::scoped_lock<std::recursive_mutex> rcLock(executionMutex);
        _files.insert(std::make_pair(file, newFile));
        m_luaState->LoadFile(file, true);
    }
}