#include "events/EventMapBlockChange.h"

constexpr EventMapBlockChange::DescriptorType EventMapBlockChange::descriptor;

EventMapBlockChange::EventMapBlockChange() {
    this->PushLua = std::bind(&EventMapBlockChange::Push, this, std::placeholders::_1);
}

int EventMapBlockChange::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, playerNumber);
    lua_pushinteger(L, mapId);
    lua_pushinteger(L, X);
    lua_pushinteger(L, Y);
    lua_pushinteger(L, Z);
    lua_pushinteger(L, bType);
    lua_pushboolean(L, undo);
    lua_pushboolean(L, physic);
    lua_pushboolean(L, send);
    lua_pushinteger(L, priority);
    
    return 11;
}

Event::DescriptorType EventMapBlockChange::type() const {
    return descriptor;
}