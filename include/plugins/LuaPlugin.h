#ifndef D3PP_LUA_PLUGIN_H
#define D3PP_LUA_PLUGIN_H

#include <string>
#include <map>
#include <memory>
#include <EventSystem.h>

#include "common/TaskScheduler.h"

struct lua_State;

namespace D3PP::plugins {
    class PluginManager;
    class LuaState;
}

struct LuaFile {
    std::string FilePath;
    time_t LastLoaded;
};

struct LuaEvent {
    std::string functionName;
    Event::DescriptorType type;
    clock_t lastRun;
    long duration;
    int mapId;
};

class LuaPlugin : TaskItem {
public:
    explicit LuaPlugin(const std::string& folder);
    ~LuaPlugin();

    void Load();
    void Unload();

    void TriggerCommand(const std::string& function, int clientId, const std::string& parsedCmd, const std::string &text0, const std::string &text1, const std::string& op1, const std::string &op2, const std::string &op3, const std::string &op4, const std::string &op5);
    void TriggerMapFill(int mapId, int sizeX, int sizeY, int sizeZ, const std::string& function, const std::string& args);
    void TriggerPhysics(int mapId, unsigned short X, unsigned short Y, unsigned short Z, const std::string& function);
    void TriggerBuildMode(const std::string &function, int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z, unsigned char mode, unsigned char block);
    void TriggerBlockCreate(const std::string& function, int mapId, unsigned short X, unsigned short Y, unsigned short Z);
    void TriggerBlockDelete(const std::string& function, int mapId, unsigned short X, unsigned short Y, unsigned short Z);
    std::string GetFolderName();
    bool IsLoaded();

private:
    std::string m_status;
    std::string m_folder;
    std::shared_ptr<D3PP::plugins::LuaState> m_luaState;
    bool m_loaded;
    std::recursive_mutex executionMutex;

    std::map<std::string, LuaFile> _files;


    static void Init();
    void TimerMain();
    void MainFunc();
    void LoadNewOrChanged();
    // -- Lua interface functions :)
    
    // -- Event executors
    void LuaDoEventTimer();

    void RegisterEventListener();
    void HandleEvent(Event &event);
    friend class D3PP::plugins::PluginManager;
};

#endif