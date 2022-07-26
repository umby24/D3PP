#include "lua/entity.h"

#include <lua.hpp>

#include "Utils.h"
#include "common/Logger.h"
#include "network/NetworkClient.h"
#include "world/Player.h"
#include "world/Entity.h"
#include "common/Player_List.h"
#include "plugins/LuaPlugin.h"

using namespace D3PP::Common;

const struct luaL_Reg LuaEntityLib::lib[] = {
       {"getall",  &LuaEntityGetTable},
       {"create", &LuaEntityAdd},
       {"delete", &LuaEntityDelete},
       {"getplayer",  &LuaEntityGetPlayer},
       {"getmap", &LuaEntityGetMapId},
       {"getposition", &LuaEntityGetPosition},
       {"getrotation", &LuaEntityGetRotation},
       {"getlook", &LuaEntityGetLook},
       {"resend", &LuaEntityResend},
       {"sendmessage", &LuaEntityMessage2Clients},
       {"getdisplayname", &LuaEntityDisplaynameGet},
       {"setdisplayname", &LuaEntityDisplaynameSet},
       {"setposition", &LuaEntityPositionSet},
       {"kill", &LuaEntityKill},
       {"setmodel", &LuaEntitySetModel},
       {NULL, NULL}
};

int LuaEntityLib::openLib(lua_State* L)
{
    lua_getglobal(L, "Entity");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, LuaEntityLib::lib, 0);
    lua_setglobal(L, "Entity");
    return 1;
}

int LuaEntityLib::LuaEntityGetTable(lua_State* L) {
    int numEntities = static_cast<int>(Entity::AllEntities.size());
    int index = 1;

    lua_newtable(L);

    if (numEntities > 0) {
        for (auto const& e : Entity::AllEntities) {
            lua_pushinteger(L, index++);
            lua_pushinteger(L, e.first);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, numEntities);

    return 2;
}

int LuaEntityLib::LuaEntityAdd(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 7) {
        Logger::LogAdd("Lua", "LuaError: Entity.Add called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    std::string eName(luaL_checkstring(L, 1));
    int mapId = static_cast<int>(luaL_checkinteger(L, 2));
    auto x = static_cast<float>(luaL_checknumber(L, 3));
    auto y = static_cast<float>(luaL_checknumber(L, 4));
    auto z = static_cast<float>(luaL_checknumber(L, 5));
    auto rotation = static_cast<float>(luaL_checknumber(L, 6));
    auto look = static_cast<float>(luaL_checknumber(L, 7));
    std::shared_ptr<Entity> newEntity = std::make_shared<Entity>(eName, mapId, x, y, z, rotation, look);
    Entity::Add(newEntity);
    int result = newEntity->Id;

    lua_pushinteger(L, result);

    return 1;
}

int LuaEntityLib::LuaEntityDelete(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Entity_Delete called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int entityId = static_cast<int>(luaL_checkinteger(L, 1));
    std::shared_ptr<Entity> e = Entity::GetPointer(entityId);

    if (e == nullptr) {
        return 0;
    }

    if (e->associatedClient != nullptr) {
        Logger::LogAdd("Lua", "LuaError: Cannot delete a player's entity.", LogType::WARNING, GLF);
        return 0;
    }

    Entity::Delete(entityId);
    return 0;
}

int LuaEntityLib::LuaEntityGetPlayer(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_PLAYER called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int entityId = static_cast<int>(luaL_checkinteger(L, 1));
    int result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr && foundEntity->playerList != nullptr) {
        result = foundEntity->playerList->Number;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaEntityLib::LuaEntityGetMapId(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_Map_Id called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int entityId = luaL_checkinteger(L, 1);
    int result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->MapID;
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaEntityLib::LuaEntityGetPosition(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Entity.GetPosition called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int entityId = luaL_checkinteger(L, 1);
    float resultX = -1;
    float resultY = -1;
    float resultZ = -1;
    
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
        auto floatCoords = foundEntity->Location.GetAsFloatCoords();
        resultX = floatCoords.X;
        resultY = floatCoords.Y;
        resultZ = floatCoords.Z;
    }

    lua_pushnumber(L, resultX);
    lua_pushnumber(L, resultY);
    lua_pushnumber(L, resultZ);
    return 3;
}

int LuaEntityLib::LuaEntityGetRotation(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_Rotation called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int entityId = luaL_checkinteger(L, 1);
    float result = -1;
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
        result = foundEntity->Location.Rotation;
    }

    lua_pushnumber(L, result);
    return 1;
}

int LuaEntityLib::LuaEntityGetLook(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_GET_Look called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    const int entityId = luaL_checkinteger(L, 1);
    float result = -1;
    const std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        result = foundEntity->Location.Look;
    }

    lua_pushnumber(L, result);
    return 1;
}

int LuaEntityLib::LuaEntityResend(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Entity_Resend called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int entityId = luaL_checkinteger(L, 1);
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
        foundEntity->Resend(entityId);
    }

    return 0;
}

int LuaEntityLib::LuaEntityMessage2Clients(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Entity_Message_2_Clients called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int entityid = luaL_checkinteger(L, 1);
    std::string message(luaL_checkstring(L, 2));

    Entity::MessageToClients(entityid, message);
    return 0;
}

int LuaEntityLib::LuaEntityDisplaynameGet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_displayname_get called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int entityId = luaL_checkinteger(L, 1);
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
        lua_pushstring(L, foundEntity->Prefix.c_str());
        lua_pushstring(L, foundEntity->Name.c_str());
        lua_pushstring(L, foundEntity->Suffix.c_str());
    }

    return 3;
}

int LuaEntityLib::LuaEntityDisplaynameSet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4 && nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_displayname_set called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int entityId = luaL_checkinteger(L, 1);
    std::string prefix = "";
    std::string displayName = "";
    std::string suffix = "";

    if (nArgs == 4) {
        prefix = std::string(luaL_checkstring(L, 2));
        displayName = std::string(luaL_checkstring(L, 3));
        suffix = std::string(luaL_checkstring(L, 4));
    } else {
        displayName = std::string(luaL_checkstring(L, 2));
    }

    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
        Entity::SetDisplayName(entityId, prefix, displayName, suffix);
        foundEntity->Resend(entityId);
    }

    return 0;
}

int LuaEntityLib::LuaEntityPositionSet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 7) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_Position_Set called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int entityId = luaL_checkinteger(L, 1);
    int mapId = luaL_checkinteger(L, 2);
    float X = luaL_checknumber(L, 3);
    float Y = luaL_checknumber(L, 4);
    float Z = luaL_checknumber(L, 5);
    float rotation = luaL_checknumber(L, 6);
    float look = luaL_checknumber(L, 7);

    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);
    if (foundEntity != nullptr) {
        MinecraftLocation newLoc{ rotation, look };
        Vector3S blockCoords{ static_cast<short>(X), static_cast<short>(Y), static_cast<short>(Z) };
        newLoc.SetAsBlockCoords(blockCoords);
        foundEntity->PositionSet(mapId, newLoc, 10, true);
    }

    return 0;
}

int LuaEntityLib::LuaEntityKill(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: ENTITY_Kill called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int entityId = luaL_checkinteger(L, 1);
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
        foundEntity->Kill();
    }

    return 0;
}

int LuaEntityLib::LuaEntitySetModel(lua_State *L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Entity.SetModel called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int entityId = static_cast<int>(luaL_checkinteger(L, 1));
    std::string modelName(luaL_checkstring(L, 2));
    std::shared_ptr<Entity> foundEntity = Entity::GetPointer(entityId);

    if (foundEntity != nullptr) {
        foundEntity->SetModel(modelName);
    }

    return 0;
}
