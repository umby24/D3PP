//
// Created by unknown on 10/29/21.
//

#include "events/PlayerEventArgs.h"

PlayerEventArgs::PlayerEventArgs(const DescriptorType* inDescript) {
    descriptor = inDescript;
}

int PlayerEventArgs::Push(lua_State *L) {
    // nop
    return 0;
}

Event::DescriptorType PlayerEventArgs::type() const {
    return *descriptor;
}

int PlayerClickEventArgs::Push(lua_State *L) {
    lua_pushinteger(L, playerId);
    lua_pushinteger(L, (int)button);
    lua_pushinteger(L, (int)action);
    lua_pushinteger(L, yaw);
    lua_pushinteger(L, pitch);
    lua_pushinteger(L, targetEntity);
    lua_pushinteger(L, targetBlock.X);
    lua_pushinteger(L, targetBlock.Y);
    lua_pushinteger(L, targetBlock.Z);
    lua_pushinteger(L, (int)blockFace);

    return 10;
}
