#include "events/EventMapActionSave.h"

constexpr EventMapActionSave::DescriptorType EventMapActionSave::descriptor;

EventMapActionSave::EventMapActionSave() {
    this->PushLua = std::bind(&EventMapActionSave::Push, this, std::placeholders::_1);
}

int EventMapActionSave::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, activeId);
    lua_pushinteger(L, mapId);
    return 3;
}

Event::DescriptorType EventMapActionSave::type() const {
    return descriptor;
}