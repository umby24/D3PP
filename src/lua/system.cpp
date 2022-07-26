#include "lua/system.h"

#include <lua.hpp>
#include <utility>
#include "common/Logger.h"
#include "Utils.h"
#include "common/Files.h"
#include "common/Configuration.h"
#include "network/Network_Functions.h"
#include "EventSystem.h"
#include "plugins/PluginManager.h"
#include "plugins/LuaPlugin.h"
#include "plugins/LuaState.h"
#include "System.h"
#include "Command.h"

int eAdd(lua_State* L) {
    LuaSystemLib * ptr = *static_cast<LuaSystemLib**>(lua_getextraspace(L));
    return ((*ptr).LuaEventAdd)(L);
}

int eDel(lua_State* L) {
    LuaSystemLib * ptr = *static_cast<LuaSystemLib**>(lua_getextraspace(L));
    return ((*ptr).LuaEventDelete)(L);
}

const struct luaL_Reg LuaSystemLib::lib[] = {
        {"msgAll", &LuaMessageToAll},
        {"msg", &LuaMessage},
        {"getfile", &LuaFileGet},
        {"getfolder", &LuaFolderGet},
        {"addEvent", &eAdd},
        {"deleteEvent", &eDel},
        {"log", &LuaSystemLog},
        {"getplatform", &LuaGetPlatform},
        {"addCmd", &LuaAddCommand},
        {"setSoftwareName", &LuaSetSoftwareName},
        {"setServerName", &LuaSetServerName},
        {"addTextColor", &LuaAddTextColor},
        {NULL, NULL}
};

int LuaSystemLib::openLib(lua_State* L, std::shared_ptr<D3PP::plugins::LuaState> thisPlugin)
{
    m_thisPlugin = std::move(thisPlugin);
    *static_cast<LuaSystemLib**>(lua_getextraspace(m_thisPlugin->GetState())) = this;

    lua_getglobal(L, "System");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, LuaSystemLib::lib, 0);
    lua_setglobal(L, "System");
    return 1;
}

int LuaSystemLib::LuaFileGet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Files_File_Get called with invalid number of arguments.", LogType::WARNING,GLF);
        return 0;
    }

    std::string fileName(luaL_checkstring(L, 1));
    lua_pushstring(L, Files::GetFile(fileName).c_str());
    return 1;
}


int LuaSystemLib::LuaFolderGet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Files_Folder_Get called with invalid number of arguments.", LogType::WARNING,GLF);
        return 0;
    }

    std::string fileName(luaL_checkstring(L, 1));
    lua_pushstring(L, Files::GetFolder(fileName).c_str());
    return 1;
}

int LuaSystemLib::LuaSystemLog(lua_State* L)
{
    int nArgs = lua_gettop(L);

    if (nArgs < 1 || nArgs > 3) {
        Logger::LogAdd("Lua", "LuaError: Log called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    std::string logMessage(luaL_checkstring(L, 1));
    std::string logType(luaL_optstring(L, 2, "info"));
    LogType thisLogType;

    if (Utils::InsensitiveCompare("info", logType))
        thisLogType = LogType::NORMAL;
    
    if (Utils::InsensitiveCompare("warning", logType))
        thisLogType = LogType::WARNING;
    
    if (Utils::InsensitiveCompare("error", logType))
        thisLogType = LogType::L_ERROR;
    
    Logger::LogAdd("Lua", logMessage, thisLogType, GLF);
    return 0;
}

int LuaSystemLib::LuaGetPlatform(lua_State* L)
{
    int nArgs = lua_gettop(L);

    if (nArgs != 0) {
        Logger::LogAdd("Lua", "LuaError: GetPlatform called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

#ifdef __linux__
    lua_pushstring(L, "linux");
#else
#ifdef MSVC
    lua_pushstring(L, "windows");
#else
    lua_pushstring(L, "windows");
#endif
#endif

    return 1;
}



int LuaSystemLib::LuaEventAdd(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 6) {
        Logger::LogAdd("Lua", "LuaError: Event_Add called with invalid number of arguments.", LogType::WARNING,GLF);
        return 0;
    }

    std::string eventId(luaL_checkstring(L, 1));
    std::string function(luaL_checkstring(L, 2));
    std::string type(luaL_checkstring(L, 3));
    int setOrCheck = luaL_checkinteger(L, 4);
    int timed = luaL_checkinteger(L, 5);
    int mapId = luaL_checkinteger(L, 6);

    if (!Dispatcher::hasDescriptor(type)) {
        Logger::LogAdd("Lua", "LuaError: Invalid event type: " + type + ".", LogType::WARNING,GLF);
        return 0;
    }

    auto typeAsEvent = Dispatcher::getDescriptor(type);

    LuaEvent newEvent{
            function,
            typeAsEvent,
            clock(),
            timed,
            mapId
    };

    if (setOrCheck == 1) {
        if (m_thisPlugin->events.find(typeAsEvent) == m_thisPlugin->events.end())
            m_thisPlugin->events.insert(std::make_pair(typeAsEvent, std::map<std::string, LuaEvent>()));

        if (m_thisPlugin->events[typeAsEvent].find(eventId) != m_thisPlugin->events[typeAsEvent].end()) {
            m_thisPlugin->events[typeAsEvent][eventId] = newEvent;
        } else {
            m_thisPlugin->events[typeAsEvent].insert(std::make_pair(eventId, newEvent));
        }
    }
    else {
        bool eventExists = false;

        if (m_thisPlugin->events.find(typeAsEvent) != m_thisPlugin->events.end()) {
            for (const auto& i : m_thisPlugin->events[typeAsEvent]) {
                if (i.second.type == typeAsEvent && i.second.functionName == function && i.first == eventId) {
                    eventExists = true;
                    break;
                }
            }
        }

        lua_pushboolean(L, eventExists);
        return 1;
    }

    return 0;
}

int LuaSystemLib::LuaEventDelete(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: System.deleteEvent called with invalid number of arguments.", LogType::WARNING,GLF);
        return 0;
    }

    std::string eventId(luaL_checkstring(L, 1));

    for(const auto& i : m_thisPlugin->events) {
        if (m_thisPlugin->events[i.first].contains(eventId))
            m_thisPlugin->events[i.first].erase(eventId);
    }

    return 0;
}

int LuaSystemLib::LuaMessageToAll(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs < 2) {
        Logger::LogAdd("Lua", "LuaError: System_Message_Network_Send_2_All called with invalid number of arguments.", LogType::WARNING,GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    std::string message(luaL_checkstring(L, 2));

    int messageType = 0;

    if (nArgs == 3 && lua_isnumber(L, 3)) {
        messageType = luaL_checkinteger(L, 3);
    }

    NetworkFunctions::SystemMessageNetworkSend2All(mapId, message, messageType);
    return 0;
}

int LuaSystemLib::LuaMessage(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs < 2) {
        Logger::LogAdd("Lua", "LuaError: System.msg called with invalid number of arguments.", LogType::WARNING,GLF);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    std::string message = luaL_checkstring(L, 2);
    int messageType = luaL_optinteger(L, 3, 0);

    if (clientId != -200) {
        // -- Verify this is a valid client id.

    }

    NetworkFunctions::SystemMessageNetworkSend(clientId, message, messageType);
    return 0;
}

int LuaSystemLib::LuaAddCommand(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs < 2) {
        Logger::LogAdd("Lua", "LuaError: System.addCmd called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    std::string commandName(luaL_checkstring(L, 1));
    std::string commandGroup(luaL_checkstring(L, 2));
    int minRank = luaL_checkinteger(L, 3);
    std::string handleFunction(luaL_checkstring(L, 4));
    std::string description(luaL_checkstring(L, 5));

    if (!handleFunction.starts_with("Lua:"))
        handleFunction = "Lua:" + handleFunction;

    CommandMain* cm = CommandMain::GetInstance();
    std::erase_if(cm->Commands, [&commandName](const Command &c){ return c.Id == commandName; });

    Command newCmd;
    newCmd.Id = commandName;
    newCmd.Name = commandName;
    newCmd.Rank = minRank;
    newCmd.RankShow = minRank;
    newCmd.Plugin = handleFunction;
    newCmd.Group = commandGroup;
    newCmd.Description = description;
    newCmd.Hidden = false;
    newCmd.Internal = false;
    newCmd.CanConsole = true;
    cm->Commands.push_back(newCmd);
    cm->RefreshGroups();

    return 0;
}

int LuaSystemLib::LuaSetSoftwareName(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs < 1) {
        Logger::LogAdd("Lua", "LuaError: System.setSoftwareName called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    std::string softwareName(luaL_checkstring(L, 1));
    System::ServerName = softwareName;

    return 0;
}

int LuaSystemLib::LuaSetServerName(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs < 1) {
        Logger::LogAdd("Lua", "LuaError: System.setServerName called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    std::string softwareName(luaL_checkstring(L, 1));
    Configuration::GenSettings.name = softwareName;
    Configuration* cc = Configuration::GetInstance();
    cc->Save();


    return 0;
}

int LuaSystemLib::LuaAddTextColor(lua_State* L)
{
    int nArgs = lua_gettop(L);

    if (nArgs != 5) {
        Logger::LogAdd("Lua", "LuaError: System.addTextColor called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    std::string character(luaL_checkstring(L, 1));
    int redVal = luaL_checkinteger(L, 2);
    int greenVal = luaL_checkinteger(L, 3);
    int blueVal = luaL_checkinteger(L, 4);
    int alphaVal = luaL_checkinteger(L, 5);

    if (character.size() > 1) {
        Logger::LogAdd("Lua", "LuaError: System.addTextColor, character invalid: may only be one character!", LogType::WARNING, GLF);
        return 0;
    }

    std::erase_if(Configuration::textSettings.colors, [&character](const CustomColor& c) { return c.character == character; });

    Configuration::textSettings.colors.push_back(CustomColor{ character, redVal, greenVal, blueVal, alphaVal });

    return 0;
}
