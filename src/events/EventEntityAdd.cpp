#include "events/EventEntityAdd.h"

EventEntityAdd::EventEntityAdd() {
    entityId = -1;
    this->PushLua = [this](auto && PH1) { return Push(std::forward<decltype(PH1)>(PH1)); };
}

int EventEntityAdd::Push(lua_State* L) const {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, entityId);
    return 2;
}

Event::DescriptorType EventEntityAdd::type() const {
    return descriptor;
}

EventEntityAdd::EventEntityAdd(const EventEntityAdd &in) : Event(in) {
    this->entityId = in.entityId;
    this->PushLua = [this](auto && PH1) { return Push(std::forward<decltype(PH1)>(PH1)); };
}
