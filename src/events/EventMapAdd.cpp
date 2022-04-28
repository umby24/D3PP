#include "events/EventMapAdd.h"

constexpr EventMapAdd::DescriptorType EventMapAdd::descriptor;

EventMapAdd::EventMapAdd() {
    this->PushLua = std::bind(&EventMapAdd::Push, this, std::placeholders::_1);
}

int EventMapAdd::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, mapId);
    return 2;
}

Event::DescriptorType EventMapAdd::type() const {
    return descriptor;
}