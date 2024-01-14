#include "events/EventMapActionResize.h"

EventMapActionResize::EventMapActionResize() {
    this->PushLua = std::bind(&EventMapActionResize::Push, this, std::placeholders::_1);
}

int EventMapActionResize::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, actionId);
    lua_pushinteger(L, mapId);
    return 3;
}

Event::DescriptorType EventMapActionResize::type() const {
    return descriptor;
}

EventMapActionResize::EventMapActionResize(const EventMapActionResize &in) : Event(in) {
    this->actionId = in.actionId;
    this->mapId = in.mapId;
    this->PushLua = std::bind(&EventMapActionResize::Push, this, std::placeholders::_1);
}
