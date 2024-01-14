#include "events/EventMapAdd.h"

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

EventMapAdd::EventMapAdd(const EventMapAdd &in) : Event(in) {
    this->mapId = in.mapId;
    this->PushLua = std::bind(&EventMapAdd::Push, this, std::placeholders::_1);
}
