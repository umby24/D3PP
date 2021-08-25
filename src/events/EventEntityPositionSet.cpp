#include "events/EventEntityPositionSet.h"

constexpr EventEntityPositionSet::DescriptorType EventEntityPositionSet::descriptor;

EventEntityPositionSet::EventEntityPositionSet() {
    this->PushLua = std::bind(&EventEntityPositionSet::Push, this, std::placeholders::_1);
}

int EventEntityPositionSet::Push(lua_State* L) {
    lua_pushinteger(L, static_cast<lua_Integer>(1));
    lua_pushinteger(L, static_cast<lua_Integer>(entityId));
    lua_pushinteger(L, static_cast<lua_Integer>(mapId));
    lua_pushnumber(L, static_cast<lua_Number>(x));
    lua_pushnumber(L, static_cast<lua_Number>(y));
    lua_pushnumber(L, static_cast<lua_Number>(z));
    lua_pushnumber(L, static_cast<lua_Number>(rotation));
    lua_pushnumber(L, static_cast<lua_Number>(look));
    lua_pushinteger(L, static_cast<lua_Integer>(priority));
    lua_pushinteger(L, static_cast<lua_Integer>(sendOwnClient));
    return 10;
}

Event::DescriptorType EventEntityPositionSet::type() const {
    return descriptor;
}