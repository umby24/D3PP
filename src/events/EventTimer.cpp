#include "events/EventTimer.h"

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

EventTimer::EventTimer(const EventTimer &in) : Event(in) {
    this->mapId = in.mapId;
    this->PushLua = std::bind(&EventTimer::Push, this, std::placeholders::_1);
}
