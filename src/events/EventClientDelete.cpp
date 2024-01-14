#include "events/EventClientDelete.h"

EventClientDelete::EventClientDelete() {
    clientId = -1;
    this->PushLua = [this](auto && PH1) { return Push(std::forward<decltype(PH1)>(PH1)); };
}

int EventClientDelete::Push(lua_State* L) const {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, clientId);
    return 2;
}

Event::DescriptorType EventClientDelete::type() const {
    return descriptor;
}

EventClientDelete::EventClientDelete(const EventClientDelete &in) : Event(in) {
    this->clientId = in.clientId;
    this->PushLua = [this](auto && PH1) { return Push(std::forward<decltype(PH1)>(PH1)); };
}
