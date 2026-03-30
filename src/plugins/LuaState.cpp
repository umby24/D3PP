//
// Created by Wande on 2/10/2022.
//

#include "plugins/LuaState.h"
#include <lua.hpp>
#include <common/Logger.h>
#include <Utils.h>

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

D3PP::plugins::LuaState::LuaState(const std::string& moduleName) {
    m_name = moduleName;
    m_state = nullptr;
}

void D3PP::plugins::LuaState::Create() {
    if (m_state != nullptr)
        return;

    m_state = luaL_newstate();
    luaL_openlibs(m_state);
    // -- TODO: Perhaps need to track this using a global lua state tracker.
}

void D3PP::plugins::LuaState::Close() {
    if (m_state == nullptr)
        return;

    lua_close(m_state);
    m_state = nullptr;
}

void D3PP::plugins::LuaState::RegisterApiLibs(std::shared_ptr<LuaState> plugin) {
    LuaClientLib cLib;
    LuaBuildModeLib bmLib;
    LuaBuildLib bLib;
    LuaEntityLib eLib;
    LuaPlayerLib pLib;
    LuaMapLib mLib;
    LuaRankLib rLib;
    m_syslib = std::make_unique<LuaSystemLib>();
    LuaNetworkLib nLib;
    LuaBlockLib blLib;
    LuaTeleporterLib tpLib;
    cLib.openLib(m_state);
    bmLib.openLib(m_state);
    bLib.openLib(m_state);
    eLib.openLib(m_state);
    pLib.openLib(m_state);
    mLib.openLib(m_state);
    LuaCPELib::openLib(m_state);
    rLib.openLib(m_state);
    // -- need
    m_syslib->openLib(m_state, plugin);
    nLib.openLib(m_state);
    blLib.openLib(m_state);
    tpLib.openLib(m_state);
}

void D3PP::plugins::LuaState::LoadString(const std::string& strToLoad) {
    int loadResult = luaL_loadstring(m_state, strToLoad.c_str());
    if (loadResult != 0) {
        Logger::LogAdd("LuaState", "Failed to load string! Subsystem: [" + m_name + "], Error: " + lua_tostring(m_state, -1), L_ERROR, GLF);
        lua_pop(m_state, 1);
        return;
    }

    loadResult = lua_pcall(m_state, 0, LUA_MULTRET, 0);
    if (loadResult != 0) {
        Logger::LogAdd("LuaState", "Failed to execute string! Subsystem: [" + m_name + "], Error: " + lua_tostring(m_state, -1), L_ERROR, GLF);
        lua_pop(m_state, 1);
        return;
    }
}

void D3PP::plugins::LuaState::LoadFile(const std::string& filepath, bool logWarnings) {
    if (m_state == NULL) {
        Logger::LogAdd("LuaState", "Failed to load file [" + filepath + "]; Lua State is NULL!", L_ERROR, GLF);
        return;
    }

    int loadResult = luaL_loadfile(m_state, filepath.c_str());
    if (loadResult != LUA_OK) {
        Logger::LogAdd("LuaState", "Failed to load file [" + filepath + "]; Lua Error! Subsystem: [" + m_name + "], Error: " + lua_tostring(m_state, -1), L_ERROR, GLF);
        lua_pop(m_state, 1);
        return;
    }

    // -- Execute the file's global functions.
    int executeResult = lua_pcall(m_state, 0, 0, 0);
    if (executeResult != LUA_OK) {
        Logger::LogAdd("LuaState", "Failed to load file [" + filepath + "]; File Initialization Error! Subsystem: [" + m_name + "], Error: " + lua_tostring(m_state, -1), L_ERROR, GLF);
        lua_pop(m_state, 1);
        return;
    }
}

D3PP::plugins::LuaState::~LuaState() {
    if (m_state != nullptr) {
        Close();
    }
}
