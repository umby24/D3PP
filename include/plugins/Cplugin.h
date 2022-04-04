//
// Created by Wande on 3/29/2022.
//

#ifndef D3PP_CPLUGIN_H
#define D3PP_CPLUGIN_H

#include <string>
#include <map>
#include <memory>
#include <EventSystem.h>

#include "common/TaskScheduler.h"

struct lua_State;
typedef bool(*CPluginInitFunc)();
typedef void(*bindFunc)(void*);

namespace D3PP::plugins {
    class PluginManager;
    class LuaState;
}

class Cplugin : TaskItem {
public:
    explicit Cplugin(std::string folder);

    void Load();
    void Unload();

    void TriggerCommand(const std::string& function, int clientId, const std::string& parsedCmd, const std::string &text0, const std::string &text1, const std::string& op1, const std::string &op2, const std::string &op3, const std::string &op4, const std::string &op5);
    void TriggerMapFill(int mapId, int sizeX, int sizeY, int sizeZ, const std::string& function, const std::string& args);
    void TriggerPhysics(int mapId, unsigned short X, unsigned short Y, unsigned short Z, const std::string& function);
    void TriggerBuildMode(const std::string &function, int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z, unsigned char mode, unsigned char block);
    std::string GetFolderName();
    bool IsLoaded();

private:
    std::string m_status;
    std::string m_folder;
    std::vector<int> m_eventDescriptors;
    bool m_loaded;
    std::recursive_mutex executionMutex;
    bool LibLoad(std::string libraryPath, void** lib);
    bool LibGetSym(void *lib, std::string sname, void *sym);
    static void Init();
    void TimerMain();
    void MainFunc();
    // -- Lua interface functions :)

    // -- Event executors
    void CDoEventTimer();

    void RegisterEventListener();
    void HandleEvent(const Event &event);
    friend class D3PP::plugins::PluginManager;
};

#endif //D3PP_CPLUGIN_H
