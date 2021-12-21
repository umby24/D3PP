#ifndef D3PP_LUA_PLUGIN_H
#define D3PP_LUA_PLUGIN_H

#include <string>
#include <map>
#include <EventSystem.h>

#include "common/TaskScheduler.h"

struct lua_State;

struct LuaFile {
    std::string FilePath;
    time_t LastLoaded;
};

struct LuaEvent {
    std::string functionName;
    Event::DescriptorType type;
    clock_t lastRun;
    long duration;
};

class LuaPlugin : TaskItem {
public:
    LuaPlugin();
    ~LuaPlugin();

    static LuaPlugin* Instance;
    static LuaPlugin* GetInstance();
    void TriggerCommand(const std::string& function, int clientId, const std::string& parsedCmd, const std::string &text0, const std::string &text1, const std::string& op1, const std::string &op2, const std::string &op3, const std::string &op4, const std::string &op5);
    void TriggerMapFill(int mapId, int sizeX, int sizeY, int sizeZ, const std::string& function, const std::string& args);
    void TriggerPhysics(int mapId, unsigned short X, unsigned short Y, unsigned short Z, const std::string& function);
    void TriggerBuildMode(const std::string &function, int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z, unsigned char mode, unsigned char block);

    std::map<Event::DescriptorType, std::vector<LuaEvent>> events;
private:
    std::recursive_mutex executionMutex;
    lua_State* state;
    std::map<std::string, LuaFile> _files;


    static void Init();
    void TimerMain();
    void MainFunc();
    void BindFunctions();
    void LoadFile(const std::string& path);
    // -- Lua interface functions :)
    
    // -- Event executors
    void LuaDoEventTimer();

    void RegisterEventListener();

    void HandleEvent(const Event &event);
};

#endif