#include "events/EventEntityAdd.h"

constexpr EventEntityAdd::DescriptorType EventEntityAdd::descriptor;

EventEntityAdd::EventEntityAdd() {
    this->PushLua = std::bind(&EventEntityAdd::Push, this, std::placeholders::_1);
}

int EventEntityAdd::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, entityId);
    return 2;
}

Event::DescriptorType EventEntityAdd::type() const {
    return descriptor;
}