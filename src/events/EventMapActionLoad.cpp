#include "events/EventMapActionLoad.h"

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

EventMapActionLoad::EventMapActionLoad(const EventMapActionLoad &in) : Event(in) {
    this->actionId = in.actionId;
    this->mapId = in.mapId;
    this->PushLua = std::bind(&EventMapActionLoad::Push, this, std::placeholders::_1);
}
