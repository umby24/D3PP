#include "events/EventMapActionDelete.h"

EventMapActionDelete::EventMapActionDelete() {
    this->PushLua = std::bind(&EventMapActionDelete::Push, this, std::placeholders::_1);
}

int EventMapActionDelete::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, actionId);
    lua_pushinteger(L, mapId);
    return 3;
}

Event::DescriptorType EventMapActionDelete::type() const {
    return descriptor;
}

EventMapActionDelete::EventMapActionDelete(const EventMapActionDelete &in) : Event(in) {
    this->actionId = in.actionId;
    this->mapId = in.mapId;
    this->PushLua = std::bind(&EventMapActionDelete::Push, this, std::placeholders::_1);
}