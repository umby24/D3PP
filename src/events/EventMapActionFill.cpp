#include "events/EventMapActionFill.h"

EventMapActionFill::EventMapActionFill() {
    this->PushLua = std::bind(&EventMapActionFill::Push, this, std::placeholders::_1);
}

int EventMapActionFill::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, actionId);
    lua_pushinteger(L, mapId);
    return 3;
}

Event::DescriptorType EventMapActionFill::type() const {
    return descriptor;
}

EventMapActionFill::EventMapActionFill(const EventMapActionFill &in) : Event(in) {
    this->actionId = in.actionId;
    this->mapId = in.mapId;
    this->PushLua = std::bind(&EventMapActionFill::Push, this, std::placeholders::_1);
}
