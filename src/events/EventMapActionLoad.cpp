#include "events/EventMapActionLoad.h"

constexpr EventMapActionLoad::DescriptorType EventMapActionLoad::descriptor;

EventMapActionLoad::EventMapActionLoad() {
    this->PushLua = std::bind(&EventMapActionLoad::Push, this, std::placeholders::_1);
}

int EventMapActionLoad::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, actionId);
    lua_pushinteger(L, mapId);
    return 3;
}

Event::DescriptorType EventMapActionLoad::type() const {
    return descriptor;
}