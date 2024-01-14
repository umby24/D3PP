#include "events/EventEntityMapChange.h"

EventEntityMapChange::EventEntityMapChange() {
    this->PushLua = std::bind(&EventEntityMapChange::Push, this, std::placeholders::_1);
}

int EventEntityMapChange::Push(lua_State* L) {
    lua_pushinteger(L, entityId);
    lua_pushinteger(L, newMapId);
    lua_pushinteger(L, oldMapId);
    return 3;
}

Event::DescriptorType EventEntityMapChange::type() const {
    return descriptor;
}

EventEntityMapChange::EventEntityMapChange(const EventEntityMapChange &in) : Event(in){
    this->entityId = in.entityId;
    this->newMapId = in.newMapId;
    this->oldMapId = in.oldMapId;
    this->PushLua = std::bind(&EventEntityMapChange::Push, this, std::placeholders::_1);
}
