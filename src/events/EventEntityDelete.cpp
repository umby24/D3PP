#include "events/EventEntityDelete.h"

EventEntityDelete::EventEntityDelete() {
    entityId = -1;
    this->PushLua = [this](auto && PH1) { return Push(std::forward<decltype(PH1)>(PH1)); };
}

int EventEntityDelete::Push(lua_State* L) const {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, entityId);
    return 2;
}

Event::DescriptorType EventEntityDelete::type() const {
    return descriptor;
}

EventEntityDelete::EventEntityDelete(const EventEntityDelete &in) : Event(in) {
    this->entityId = in.entityId;
    this->PushLua = [this](auto && PH1) { return Push(std::forward<decltype(PH1)>(PH1)); };
}
