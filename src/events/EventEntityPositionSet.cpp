#include "events/EventEntityPositionSet.h"

constexpr EventEntityPositionSet::DescriptorType EventEntityPositionSet::descriptor;

EventEntityPositionSet::EventEntityPositionSet() {
    this->PushLua = std::bind(&EventEntityPositionSet::Push, this, std::placeholders::_1);
}

int EventEntityPositionSet::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, entityId);
    lua_pushinteger(L, mapId);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);
    lua_pushnumber(L, rotation);
    lua_pushnumber(L, look);
    lua_pushinteger(L, priority);
    lua_pushboolean(L, sendOwnClient);
    return 10;
}

Event::DescriptorType EventEntityPositionSet::type() const {
    return descriptor;
}