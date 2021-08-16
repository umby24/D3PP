#include "events/EventTimer.h"

constexpr EventTimer::DescriptorType EventTimer::descriptor;

EventTimer::EventTimer() {
    this->PushLua = std::bind(&EventTimer::Push, this, std::placeholders::_1);
}

int EventTimer::Push(lua_State* L) {
    lua_pushinteger(L, mapId);
    return 1;
}

Event::DescriptorType EventTimer::type() const {
    return descriptor;
}