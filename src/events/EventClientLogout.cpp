#include "events/EventClientLogout.h"

EventClientLogout::EventClientLogout() {
    clientId = -1;
    this->PushLua = [this](auto && PH1) { return Push(std::forward<decltype(PH1)>(PH1)); };
}

int EventClientLogout::Push(lua_State* L) const {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, clientId);
    return 2;
}

Event::DescriptorType EventClientLogout::type() const {
    return descriptor;
}

EventClientLogout::EventClientLogout(const EventClientLogout &in) : Event(in) {
    this->clientId = in.clientId;
    this->PushLua = [this](auto && PH1) { return Push(std::forward<decltype(PH1)>(PH1)); };
}
