#include "events/EventEntityDie.h"

EventEntityDie::EventEntityDie() {
    this->PushLua = std::bind(&EventEntityDie::Push, this, std::placeholders::_1);
}

int EventEntityDie::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, entityId);
    return 2;
}

Event::DescriptorType EventEntityDie::type() const {
    return descriptor;
}

EventEntityDie::EventEntityDie(const EventEntityDie &in) : Event(in) {
    this->entityId = in.entityId;
    this->PushLua = std::bind(&EventEntityDie::Push, this, std::placeholders::_1);
}
