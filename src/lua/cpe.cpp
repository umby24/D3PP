#include "lua/cpe.h"

#include <lua.hpp>
#include "CPE.h"
#include "common/Logger.h"
#include "Utils.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "common/MinecraftLocation.h"
#include "world/Entity.h"
#include "world/Player.h"
#include "world/Map.h"
#include "Block.h"
#include "CustomBlocks.h"

const struct luaL_Reg LuaCPELib::lib[] = {
        {"getsrvexts", &LuaServerGetExtensions},
        {"getexts", &LuaClientGetExtension},
        {"addselection", &LuaSelectionCuboidAdd},
        {"deleteselection", &LuaSelectionCuboidDelete},
        {"getheld", &LuaGetHeldBlock},
        {"setheld", &LuaSetHeldBlock},
        {"setenvcolors", &LuaMapSetEnvColors},
        {"setblockperms", &LuaClientSetBlockPermissions},
        {"setmapenv", &LuaMapEnvSet},
        {"setclienthacks", &LuaClientHackcontrolSend},
        {"addhotkey", &LuaHotkeyAdd},
        {"removehotkey", &LuaHotkeyRemove},
        {"setmaphacks", &LuaMapHackcontrolSet},
        {"setmodel", &LuaChangeModel},
        {"setweather", &LuaSetWeather},
        {"createblockdef", &LuaCreateBlock},
        {"deleteblockdef", &LuaDeleteBlock},
        {"createclientblockdef", &LuaCreateBlockClient},
        {"deleteclientblockdef", &LuaDeleteBlockClient},
        {"setblockext", &LuaSetBlockExt},
        {"setblockextclient", &LuaSetBlockExtClient},
       {NULL, NULL}
};

int LuaCPELib::openLib(lua_State* L)
{
    lua_getglobal(L, "CPE");
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, LuaCPELib::lib, 0);
    lua_setglobal(L, "CPE");
    return 1;
}


int LuaCPELib::LuaServerGetExtensions(lua_State* L) {
    return 0;
}


int LuaCPELib::LuaClientGetExtension(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: Client_Get_Extension() called with invalid number of arguments.", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    std::string extension(luaL_checkstring(L, 2));
    int result = 0;
    Network* nm = Network::GetInstance();
    std::shared_ptr<IMinecraftClient> c = nm->GetClient(clientId);

    if (c != nullptr) {
        result = CPE::GetClientExtVersion(c, extension);
    }

    lua_pushinteger(L, result);
    return 1;
}

int LuaCPELib::LuaClientGetExtensions(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: Client_Get_Extensions called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    Network* nm = Network::GetInstance();
    auto client = std::static_pointer_cast<NetworkClient>(nm->GetClient(clientId));

    if (client == nullptr || !client->LoggedIn)
        return 0;

    int number = static_cast<int>(client->Extensions.size());
    lua_newtable(L);

    if (number > 0) {
        for (auto const& nc : client->Extensions) {
            lua_pushstring(L, nc.first.c_str());
            lua_pushinteger(L, nc.second);
            lua_settable(L, -3);
        }
    }

    lua_pushinteger(L, number);

    return 2;
}

int LuaCPELib::LuaSelectionCuboidAdd(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 13) {
        Logger::LogAdd("Lua", "LuaError: CPE_Selection_Cuboid_Add called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    int selectionId = luaL_checkinteger(L, 2);
    std::string label(luaL_checkstring(L, 3));
    Vector3S start{};
    Vector3S end{};
    start.X = luaL_checkinteger(L, 4);
    start.Y = luaL_checkinteger(L, 5);
    start.Z = luaL_checkinteger(L, 6);
    end.X = luaL_checkinteger(L, 7);
    end.Y = luaL_checkinteger(L, 8);
    end.Z = luaL_checkinteger(L, 9);
    short red = luaL_checkinteger(L, 10);
    short green = luaL_checkinteger(L, 11);
    short blue = luaL_checkinteger(L, 12);
    float opacity = luaL_checknumber(L, 13);

    Network* nm = Network::GetInstance();
    auto nc = std::static_pointer_cast<NetworkClient>(nm->GetClient(clientId));
    if (nc == nullptr)
        return 0;

    nc->CreateSelection(static_cast<unsigned char>(selectionId), label, start.X, start.Y, start.Z, end.X, end.Y, end.Z, red, green, blue, static_cast<short>(opacity));
    return 0;
}

int LuaCPELib::LuaSelectionCuboidDelete(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: CPE_Selection_Cuboid_Delete called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    int selectionId = luaL_checkinteger(L, 2);
    Network* nm = Network::GetInstance();
    auto nc = std::static_pointer_cast<NetworkClient>(nm->GetClient(clientId));

    if (nc == nullptr)
        return 0;

    nc->DeleteSelection(static_cast<unsigned char>(selectionId));
    return 0;
}

int LuaCPELib::LuaGetHeldBlock(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: CPE_Get_Held_Block called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    Network* nm = Network::GetInstance();
    std::shared_ptr<IMinecraftClient> nc = nm->GetClient(clientId);
    if (nc == nullptr)
        return 0;
    std::shared_ptr<Entity> e = Entity::GetPointer(clientId, true);

    if (e != nullptr) {
        lua_pushinteger(L, e->heldBlock);
        return 1;
    }

    return 0;
}

int LuaCPELib::LuaSetHeldBlock(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 3) {
        Logger::LogAdd("Lua", "LuaError: CPE_Set_Held_Block called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    int blockId = luaL_checkinteger(L, 2);
    int canChange = luaL_checkinteger(L, 3);

    Network* nm = Network::GetInstance();
    auto nc = std::static_pointer_cast<NetworkClient>(nm->GetClient(clientId));

    if (nc == nullptr)
        return 0;

    if (nc->LoggedIn && nc->player && nc->player->tEntity) {
        nc->HoldThis(blockId, canChange);
        return 1;
    }

    return 0;
}

int LuaCPELib::LuaChangeModel(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: CPE_Change_Model called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    std::string model(luaL_checkstring(L, 2));

    Network* nm = Network::GetInstance();
    std::shared_ptr<IMinecraftClient> nc = nm->GetClient(clientId);

    if (nc == nullptr)
        return 0;
    std::shared_ptr<Entity> e = Entity::GetPointer(clientId, true);

    if (e != nullptr) {
        e->SetModel(model);
    }

    return 0;
}

int LuaCPELib::LuaSetWeather(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: CPE_Set_Weather called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    int weatherType = luaL_checkinteger(L, 2);

    if (weatherType != 0 && weatherType != 1 && weatherType != 2)
        return 0;

    Network* nm = Network::GetInstance();
    auto nc = std::static_pointer_cast<NetworkClient>(nm->GetClient(clientId));

    if (nc == nullptr || !nc->LoggedIn)
        return 0;

    nc->SetWeather(weatherType);

    return 0;
}

int LuaCPELib::LuaMapSetEnvColors(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 5) {
        Logger::LogAdd("Lua", "LuaError: CPE_Map_Set_Env_Colors called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    int red = luaL_checkinteger(L, 2);
    int green = luaL_checkinteger(L, 3);
    int blue = luaL_checkinteger(L, 4);
    int type = luaL_checkinteger(L, 5);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> thisMap = mm->GetPointer(mapId);

    if (thisMap == nullptr)
        return 0;

    thisMap->SetEnvColors(red, green, blue, type);

    return 0;
}

int LuaCPELib::LuaClientSetBlockPermissions(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: CPE_Client_Set_Block_Permissions called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    int blockId = luaL_checkinteger(L, 2);
    bool canPlace = lua_toboolean(L, 3);
    bool canDelete = lua_toboolean(L, 4);

    Network* nm = Network::GetInstance();
    auto nc = std::static_pointer_cast<NetworkClient>(nm->GetClient(clientId));

    if (nc == nullptr || !nc->LoggedIn)
        return 0;

    nc->SetBlockPermissions(blockId, canPlace, canDelete);

    return 0;
}

int LuaCPELib::LuaMapEnvSet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: CPE_Map_Env_Apperance_Set called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    std::string customUrl(luaL_checkstring(L, 2));
    int sideBlock = luaL_checkinteger(L, 3);
    int edgeBlock = luaL_checkinteger(L, 4);
    int sideLevel = luaL_checkinteger(L, 5);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> thisMap = mm->GetPointer(mapId);

    if (thisMap == nullptr)
        return 0;

    thisMap->SetMapAppearance(customUrl, sideBlock, edgeBlock, sideLevel);
    return 0;
}

int LuaCPELib::LuaClientHackcontrolSend(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 7) {
        Logger::LogAdd("Lua", "LuaError: CPE_Client_Hackcontrol_Send called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int clientId = luaL_checkinteger(L, 1);
    bool canFly = lua_toboolean(L, 2);
    bool noclip = lua_toboolean(L, 3);
    bool speeding = lua_toboolean(L, 4);
    bool spawnControl = lua_toboolean(L, 5);
    bool thirdperson = lua_toboolean(L, 6);
    int jumpHeight = luaL_checkinteger(L, 7);

    Network* nm = Network::GetInstance();
    auto nc = std::static_pointer_cast<NetworkClient>(nm->GetClient(clientId));

    if (nc == nullptr || !nc->LoggedIn)
        return 0;

    nc->SendHackControl(canFly, noclip, speeding, spawnControl, thirdperson, jumpHeight);
    return 0;
}

int LuaCPELib::LuaHotkeyAdd(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 4) {
        Logger::LogAdd("Lua", "LuaError: CPE_Hotkey_Add called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    std::string label(luaL_checkstring(L, 1));
    std::string action(luaL_checkstring(L, 2));
    int keycode = luaL_checkinteger(L, 3);
    int keymods = luaL_checkinteger(L, 4);

    // -- TODO: Hotkey.Add
    return 0;
}

int LuaCPELib::LuaHotkeyRemove(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: CPE_Hotkey_Add called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    std::string label(luaL_checkstring(L, 1));
    // -- TODO: Hotkey.remove
    return 0;
}

int LuaCPELib::LuaMapHackcontrolSet(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 7) {
        Logger::LogAdd("Lua", "LuaError: CPE_Map_Hackcontrol_Set called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int mapId = luaL_checkinteger(L, 1);
    bool canFly = lua_toboolean(L, 2);
    bool noclip = lua_toboolean(L, 3);
    bool speeding = lua_toboolean(L, 4);
    bool spawnControl = lua_toboolean(L, 5);
    bool thirdperson = lua_toboolean(L, 6);
    int jumpHeight = luaL_checkinteger(L, 7);

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> map = mm->GetPointer(mapId);

    if (map == nullptr)
        return 0;

    map->SetHackControl(canFly, noclip, speeding, spawnControl, thirdperson, jumpHeight);
    return 0;
}

int LuaCPELib::LuaCreateBlock(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 16) {
        Logger::LogAdd("Lua", "LuaError: BlockGlobalCreate called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int blockId = luaL_checkinteger(L, 1);
    std::string blockName(luaL_checkstring(L, 2));
    int solidity = luaL_checkinteger(L, 3);
    int movementSpeed = luaL_checkinteger(L, 4);
    int topTexture = luaL_checkinteger(L, 5);
    int sideTexture = luaL_checkinteger(L, 6);
    int bottomTexture = luaL_checkinteger(L, 7);
    bool transmitsLight = lua_toboolean(L, 8);
    int walkSound = luaL_checkinteger(L, 9);
    bool fullBright = lua_toboolean(L, 10);
    int shape = luaL_checkinteger(L, 11);
    int drawType = luaL_checkinteger(L, 12);
    int fogDensity = luaL_checkinteger(L, 13);
    int fogR = luaL_checkinteger(L, 14);
    int fogG = luaL_checkinteger(L, 15);
    int fogB = luaL_checkinteger(L, 16);

    if (blockId == 0) {
        Logger::LogAdd("Lua", "LuaError: You cannot redefine the air block!", LogType::WARNING, GLF);
        return 0;
    }
    if (blockId > 255) {
        Logger::LogAdd("Lua", "LuaError: Invalid argument, blockid cannot be more than 255.", LogType::WARNING, GLF);
        return 0;
    }

    BlockDefinition newBlock{ static_cast<unsigned char>(blockId),
                               blockName,
                               static_cast<BlockSolidity>(solidity),
                               static_cast<char>(movementSpeed),
                               topTexture,
                               sideTexture,
                                sideTexture,
                                sideTexture,
                                sideTexture,
                              bottomTexture,
                               transmitsLight,
                               static_cast<char>(walkSound),
                               fullBright,
                                0,0,0,16,16,16,
                               static_cast<char>(shape),
                               static_cast<char>(drawType),
                               static_cast<char>(fogDensity),
                               static_cast<char>(fogR),
                               static_cast<char>(fogG),
                               static_cast<char>(fogB)
    };

    Block* b = Block::GetInstance();
    CustomBlocks* cb = CustomBlocks::GetInstance();

    b->Blocks[blockId].OnClient = blockId;
    b->Blocks[blockId].Name = blockName;
    b->SaveFile = true;
    cb->Add(newBlock);

    return 0;
}

int LuaCPELib::LuaDeleteBlock(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 1) {
        Logger::LogAdd("Lua", "LuaError: BlockDelete called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int blockId = luaL_checkinteger(L, 1);
    CustomBlocks* cb = CustomBlocks::GetInstance();
    cb->Remove(blockId);

    Block* b = Block::GetInstance();
    b->Blocks[blockId].OnClient = 4;
    b->Blocks[blockId].Name = "Invalid";
    b->SaveFile = true;

    return 0;
}

int LuaCPELib::LuaSetBlockExt(lua_State* L)
{
    // -- Args: blockId, topText, leftText, rightText, frontText, backText, bottomText, minX, minY, minZ, maxX, maxY, maxZ.
    int nArgs = lua_gettop(L);

    if (nArgs != 13) {
        Logger::LogAdd("Lua", "LuaError: SetBlockExt called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    
    Block* b = Block::GetInstance();
    CustomBlocks* cb = CustomBlocks::GetInstance();

    int blockId = luaL_checkinteger(L, 1);

    if (!cb->HasDef(blockId)) {
        Logger::LogAdd("Lua", "LuaError: Attempt to set block ext on non-existing block. Create it using CPE.CreateBlockDef first", LogType::WARNING, GLF);
        return 0;
    }

    int topTexture = luaL_checkinteger(L, 2);
    int leftTexture = luaL_checkinteger(L, 3);
    int rightTexture = luaL_checkinteger(L, 4);
    int frontTexture = luaL_checkinteger(L, 5);
    int backTexture = luaL_checkinteger(L, 6);
    int bottomTexture = luaL_checkinteger(L, 7);
    int minX = luaL_checkinteger(L, 8);
    int minY = luaL_checkinteger(L, 9);
    int minZ = luaL_checkinteger(L, 10);
    int maxX = luaL_checkinteger(L, 11);
    int maxY = luaL_checkinteger(L, 12);
    int maxZ = luaL_checkinteger(L, 13);

    auto def = cb->GetDef(blockId);
    def.topTexture = topTexture;
    def.leftTexture = leftTexture;
    def.rightTexture = rightTexture;
    def.frontTexture = frontTexture;
    def.backTexture = backTexture;
    def.bottomTexture = bottomTexture;
    def.minX = minX;
    def.minY = minY;
    def.minZ = minZ;
    def.maxX = maxX;
    def.maxY = maxY;
    def.maxZ = maxZ;
    cb->Add(def); // -- will upate :) 

    return 0;
}

int LuaCPELib::LuaSetBlockExtClient(lua_State* L)
{
    // -- Args: blockId, topText, leftText, rightText, frontText, backText, bottomText, minX, minY, minZ, maxX, maxY, maxZ.
    int nArgs = lua_gettop(L);

    if (nArgs != 14) {
        Logger::LogAdd("Lua", "LuaError: SetBlockExtClient called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    Block* b = Block::GetInstance();
    CustomBlocks* cb = CustomBlocks::GetInstance();

    int blockId = luaL_checkinteger(L, 1);

    if (!cb->HasDef(blockId)) {
        Logger::LogAdd("Lua", "LuaError: Attempt to set block ext on non-existing block. Create it using CPE.CreateBlockDef first", LogType::WARNING, GLF);
        return 0;
    }

    int topTexture = luaL_checkinteger(L, 2);
    int leftTexture = luaL_checkinteger(L, 3);
    int rightTexture = luaL_checkinteger(L, 4);
    int frontTexture = luaL_checkinteger(L, 5);
    int backTexture = luaL_checkinteger(L, 6);
    int bottomTexture = luaL_checkinteger(L, 7);
    int minX = luaL_checkinteger(L, 8);
    int minY = luaL_checkinteger(L, 9);
    int minZ = luaL_checkinteger(L, 10);
    int maxX = luaL_checkinteger(L, 11);
    int maxY = luaL_checkinteger(L, 12);
    int maxZ = luaL_checkinteger(L, 13);
    int clientId = luaL_checkinteger(L, 14);

    auto def = cb->GetDef(blockId);
    def.topTexture = topTexture;
    def.leftTexture = leftTexture;
    def.rightTexture = rightTexture;
    def.frontTexture = frontTexture;
    def.backTexture = backTexture;
    def.bottomTexture = bottomTexture;
    def.minX = minX;
    def.minY = minY;
    def.minZ = minZ;
    def.maxX = maxX;
    def.maxY = maxY;
    def.maxZ = maxZ;

    Network* n = Network::GetInstance();
    std::shared_ptr<IMinecraftClient> client = n->GetClient(clientId);
    if (client != nullptr) {
        client->SendDefineBlock(def);
    }

    return 0;
}

int LuaCPELib::LuaCreateBlockClient(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 17) {
        Logger::LogAdd("Lua", "LuaError: BlockCreateClient called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }
    int blockId = luaL_checkinteger(L, 1);
    std::string blockName(luaL_checkstring(L, 2));
    int solidity = luaL_checkinteger(L, 3);
    int movementSpeed = luaL_checkinteger(L, 4);
    int topTexture = luaL_checkinteger(L, 5);
    int sideTexture = luaL_checkinteger(L, 6);
    int bottomTexture = luaL_checkinteger(L, 7);
    bool transmitsLight = lua_toboolean(L, 8);
    int walkSound = luaL_checkinteger(L, 9);
    bool fullBright = lua_toboolean(L, 10);
    int shape = luaL_checkinteger(L, 11);
    int drawType = luaL_checkinteger(L, 12);
    int fogDensity = luaL_checkinteger(L, 13);
    int fogR = luaL_checkinteger(L, 14);
    int fogG = luaL_checkinteger(L, 15);
    int fogB = luaL_checkinteger(L, 16);
    int clientId = luaL_checkinteger(L, 17);

    if (blockId == 0) {
        Logger::LogAdd("Lua", "LuaError: You cannot redefine the air block!", LogType::WARNING, GLF);
        return 0;
    }

    if (blockId > 255) {
        Logger::LogAdd("Lua", "LuaError: Invalid argument, blockid cannot be more than 255.", LogType::WARNING, GLF);
        return 0;
    }

    BlockDefinition newBlock{ static_cast<unsigned char>(blockId),
                               blockName,
                               static_cast<BlockSolidity>(solidity),
                               static_cast<char>(movementSpeed),
                               static_cast<char>(topTexture),
                               static_cast<char>(sideTexture),
                                static_cast<char>(sideTexture),
                                static_cast<char>(sideTexture),
                                static_cast<char>(sideTexture),
                               static_cast<char>(bottomTexture),
                               transmitsLight,
                               static_cast<char>(walkSound),
                               fullBright,
                                0,0,0,16,16,16,
                               static_cast<char>(shape),
                               static_cast<char>(drawType),
                               static_cast<char>(fogDensity),
                               static_cast<char>(fogR),
                               static_cast<char>(fogG),
                               static_cast<char>(fogB)
    };
    Network* n = Network::GetInstance();
    std::shared_ptr<IMinecraftClient> client = n->GetClient(clientId);
    if (client != nullptr) {
        client->SendDefineBlock(newBlock);
    }
    return 0;
}

int LuaCPELib::LuaDeleteBlockClient(lua_State* L) {
    int nArgs = lua_gettop(L);

    if (nArgs != 2) {
        Logger::LogAdd("Lua", "LuaError: BlockDeleteClient called with invalid number of arguments.", LogType::WARNING, GLF);
        return 0;
    }

    int blockId = luaL_checkinteger(L, 1);
    int clientId = luaL_checkinteger(L, 2);
    Network* n = Network::GetInstance();
    std::shared_ptr<IMinecraftClient> client = n->GetClient(clientId);

    if (client != nullptr) {
        client->SendDeleteBlock(blockId);
    }

    return 0;
}