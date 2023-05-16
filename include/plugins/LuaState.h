//
// Created by Wande on 2/4/2022.
//

#ifndef D3PP_LUASTATE_H
#define D3PP_LUASTATE_H
#include <string>
#include <memory>
#include <map>
#include <shared_mutex>
#include "EventSystem.h"
#include "LuaPlugin.h"

struct lua_State;
class LuaSystemLib;

namespace D3PP::plugins {
    class LuaState {
    public:
        LuaState(const std::string& moduleName);
        ~LuaState();

        void Create();
        void Close();
        void RegisterApiLibs(std::shared_ptr<LuaState> plugin);
        // -- Potentially un-needed.
        void LoadString(const std::string& strToLoad);
        void LoadFile(const std::string& filepath, bool logWarnings);

        lua_State* GetState() { return m_state; }
        std::shared_mutex eventMutex;
        std::map<Event::DescriptorType, std::map<std::string, LuaEvent>> events;
        std::map<std::string, LuaEvent> modifyList;
    private:
        std::string m_name;
        lua_State* m_state;
        std::unique_ptr<LuaSystemLib> m_syslib;
    };
}
#endif //D3PP_LUASTATE_H
