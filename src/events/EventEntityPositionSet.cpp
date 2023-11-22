#include "events/EventEntityPositionSet.h"
#include "common/Logger.h"

EventEntityPositionSet::EventEntityPositionSet() {
    entityId = -1;
    mapId = -1;
    x = 0; y = 0; z = 0;
    rotation = 0; look = 0;
    priority = 0; sendOwnClient = false;
    this->PushLua = [this](auto && PH1) { return Push(std::forward<decltype(PH1)>(PH1)); };
}

int EventEntityPositionSet::Push(lua_State* L) const {
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

EventEntityPositionSet::EventEntityPositionSet(const EventEntityPositionSet &in) : Event(in) {
    entityId = in.entityId;
    mapId = in.mapId;
    x = in.x; y = in.y; z = in.z;
    rotation = in.rotation; look = in.look;
    priority = in.priority; sendOwnClient = in.sendOwnClient;
    this->PushLua = [this](auto && PH1) { return Push(std::forward<decltype(PH1)>(PH1)); };
}
