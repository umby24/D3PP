#include "events/EventMapActionDelete.h"

constexpr EventMapActionDelete::DescriptorType EventMapActionDelete::descriptor;

EventMapActionDelete::EventMapActionDelete() {
    this->PushLua = std::bind(&EventMapActionDelete::Push, this, std::placeholders::_1);
}

int EventMapActionDelete::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, activeId);
    lua_pushinteger(L, mapId);
    return 3;
}

Event::DescriptorType EventMapActionDelete::type() const {
    return descriptor;
}