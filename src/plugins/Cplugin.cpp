//
// Created by Wande on 3/29/2022.
//

#include "plugins/Cplugin.h"

#include <filesystem>
#include <events/PlayerEventArgs.h>

#include "world/Map.h"
#include "common/Logger.h"
#include "Utils.h"
#include "network/NetworkClient.h"
#include "Rank.h"
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

#ifdef __linux__
#define PLUGIN_EXT ".so"
#else
#define PLUGIN_EXT "dll"
#include <windows.h>
#include <libloaderapi.h>
#endif
void CPluginLogger(const char* message) {
    Logger::LogAdd("PluginLogger", std::string(message), LogType::NORMAL, GLF);
}

void Cplugin::RegisterEventListener() {
    int eventSub = 0;
    eventSub = Dispatcher::subscribe(EventChatAll{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventChatMap{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    // Dispatcher::subscribe(EventChatPrivate{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    eventSub = Dispatcher::subscribe(EventClientAdd{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventClientDelete{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventClientLogin{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventClientLogout{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventEntityAdd{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventEntityDelete{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventEntityDie{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventEntityMapChange{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventEntityPositionSet{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventMapActionDelete{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventMapActionFill{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventMapActionLoad{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventMapActionResize{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventMapActionSave{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventMapAdd{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventMapBlockChange{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventMapBlockChangeClient{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventMapBlockChangePlayer{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(EventTimer{}.type(), [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
    eventSub = Dispatcher::subscribe(PlayerClickEventArgs::clickDescriptor, [this](auto && PH1) { HandleEvent(std::forward<decltype(PH1)>(PH1)); });
    m_eventDescriptors.push_back(eventSub);
}

void Cplugin::HandleEvent(const Event& event) {
    if (!event.PushLua)
        return;

    if (!m_loaded)
        return;

    auto type = event.type(); // -- get the type..

//    if(m_luaState->events.find( type ) == m_luaState->events.end() ) // -- find all functions that want to be called on this event
//        return;
//
//    auto&& observers = m_luaState->events.at( type );
//
//    for( auto&& observer : observers ) {
//        executionMutex.lock();
//        lua_getglobal(m_luaState->GetState(), observer.functionName.c_str()); // -- Get the function to be called
//        if (!lua_isfunction(m_luaState->GetState(), -1)) {
//            lua_pop(m_luaState->GetState(), 1);
//            executionMutex.unlock();
//            continue;
//        }
//        int argCount = event.PushLua(m_luaState->GetState()); // -- Have the event push its args and return how many were pushed..
//        try {
//            if (lua_pcall(m_luaState->GetState(), argCount, 0, 0) != 0) { // -- Call the function.
//                bail(m_luaState->GetState(), "[Event Handler]"); // -- catch errors
//                executionMutex.unlock();
//                return;
//            }
//        } catch (const int exception) {
//            bail(m_luaState->GetState(), "[Error Handler]"); // -- catch errors
//            executionMutex.unlock();
//            return;
//        }
//        executionMutex.unlock();
//        // -- done.
//    }
}

void Cplugin::Init() {
}

std::vector<std::string> EnumerateDirectoryC(const std::string &dir) {
    std::vector<std::string> result{};

    if (std::filesystem::is_directory(dir)) {
        for (const auto &entry : std::filesystem::directory_iterator(dir)) {
            std::string fileName = entry.path().filename().string();

            if (fileName.length() < 3)
                continue;

            if (entry.is_directory()) {
                std::vector<std::string> recurseResult = EnumerateDirectoryC(entry.path().string());
                result.insert(result.end(), recurseResult.begin(), recurseResult.end());
            }

            if (fileName.substr(fileName.length() - 3) == PLUGIN_EXT) {
                result.push_back(entry.path().string());
            }
        }
    }

    return result;
}

void Cplugin::MainFunc() {
    if (!m_loaded)
        return;

}

void Cplugin::TimerMain() {
    auto timerDescriptor = Dispatcher::getDescriptor("Timer");
//    if (m_luaState->events.find(timerDescriptor) == m_luaState->events.end())
//        return;
//
//    auto &eventsAt = m_luaState->events[timerDescriptor];
//    for(auto &e : eventsAt) {
//        if (clock() >= (e.lastRun + e.duration)) {
//            e.lastRun = clock();
//            executionMutex.lock();
//            lua_getglobal(m_luaState->GetState(), e.functionName.c_str()); // -- Get the function to be called
//            if (!lua_isfunction(m_luaState->GetState(), -1)) {
//                lua_pop(m_luaState->GetState(), 1);
//                executionMutex.unlock();
//                continue;
//            }
//            if (!lua_checkstack(m_luaState->GetState(), 1)) {
//
//            }
//            lua_pushinteger(m_luaState->GetState(), e.mapId);
//            int result = lua_pcall(m_luaState->GetState(), 1, 0, 0);
//            if (result == LUA_ERRRUN) {
//                bail(m_luaState->GetState(), "[Timer Event Handler]" + e.functionName); // -- catch errors
//                executionMutex.unlock();
//                return;
//            }
//            else if (result == LUA_ERRMEM) {
//                bail(m_luaState->GetState(), "[Timer Event Handler]" + e.functionName); // -- catch errors
//                executionMutex.unlock();
//                return;
//            }
//            else if (result == LUA_ERRERR) {
//                bail(m_luaState->GetState(), "[Timer Event Handler]" + e.functionName); // -- catch errors
//                executionMutex.unlock();
//                return;
//            }
//            else {
//                // -- success? maybe? who knows.
//
//            }
//            executionMutex.unlock();
//        }
//    }
}

void Cplugin::TriggerMapFill(int mapId, int sizeX, int sizeY, int sizeZ, const std::string& function, const std::string& args) {
    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);

}

void Cplugin::TriggerPhysics(int mapId, unsigned short X, unsigned short Y, unsigned short Z, const std::string& function) {
    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);

}

void Cplugin::TriggerCommand(const std::string& function, int clientId, const std::string& parsedCmd, const std::string& text0,
                               const std::string& text1, const std::string& op1, const std::string& op2, const std::string& op3, const std::string& op4,
                               const std::string& op5) {
    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);

}

void Cplugin::TriggerBuildMode(const std::string& function, int clientId, int mapId, unsigned short X, unsigned short Y,
                                 unsigned short Z, unsigned char mode, unsigned char block) {
    std::scoped_lock<std::recursive_mutex> pqlock(executionMutex);

}

Cplugin::Cplugin(std::string folder) {
    m_folder = folder;
    m_loaded = false;

    this->Interval = std::chrono::seconds(2);
    this->Main = [this] { MainFunc(); };

    TaskScheduler::RegisterTask("CPlugin " + m_folder, *this);
}

void Cplugin::Load() {
    std::vector<std::string> pluginFiles = EnumerateDirectoryC(m_folder);
    if (pluginFiles.empty()) {
        Logger::LogAdd("Cplugin", "No lua files found, plugin is probably missing.", LogType::WARNING, GLF);
        return;
    }

    for (auto const& file : pluginFiles) {
        void* lib;
        int* plugApiVers;
        CPluginInitFunc plugInitFunc;
        bindFunc plugBinder;
        std::string entirePath = std::filesystem::current_path().append(file).string();

        if (LibLoad(entirePath, &lib)) { // -- Library loaded! Search for our required fields.
            bool versionResult = LibGetSym(lib, "Plugin_Load", (void *)&plugInitFunc);
            bool apiVerResult = LibGetSym(lib, "Plugin_ApiVers", (void *)&plugApiVers);
            bool plugBinderSuc = LibGetSym(lib, "Plugin_Setup", (void *)&plugBinder);

            if (!(versionResult && apiVerResult)) {
                Logger::LogAdd("Cplugin", "Error finding plugin load and version functions.", L_ERROR, GLF);

                unsigned long thatError = GetLastError();
                printf("Last Error: %lu", thatError);
                if (versionResult) {
                    plugInitFunc();
                } else if (apiVerResult) {
                    printf("%d", *plugApiVers);
                }
                Unload();
                continue;
            }
            Logger::LogAdd("CPlugin", "Maybe loaded that thing? " + file, WARNING, GLF);
            plugInitFunc();
            plugBinder(CPluginLogger);
            Logger::LogAdd("CPlugin", "Ran the init.. version is " + stringulate(*plugApiVers), WARNING, GLF);
        }
    }

    RegisterEventListener();

    m_status = "Loaded";
    m_loaded = true;
}

std::string Cplugin::GetFolderName() {
    return m_folder;
}

bool Cplugin::IsLoaded() {
    return m_status == "Loaded";
}

void Cplugin::Unload() {
    for(const int eId : m_eventDescriptors) {
        Dispatcher::unsubscribe(eId);
    }
    m_loaded = false;
    m_status = "Unloaded";
}

bool Cplugin::LibLoad(std::string libraryPath, void** lib) {
#ifdef __linux__
    return (*lib = dlopen(path, RTLD_NOW)) != NULL;
#else
    return (*lib = LoadLibraryA(libraryPath.c_str())) != NULL;
#endif
}

bool Cplugin::LibGetSym(void *lib, std::string sname, void *sym) {
#ifdef __linux__
    return (*(void **)sym = dlsym(lib, sname)) != NULL;
#else
    return (*(void **)sym = (void *)GetProcAddress((HMODULE)lib, sname.c_str())) != NULL;
#endif
}

