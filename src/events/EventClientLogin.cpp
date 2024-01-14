#include "events/EventClientLogin.h"

EventClientLogin::EventClientLogin() {
    clientId = -1;
    this->PushLua = [this](auto && PH1) { return Push(std::forward<decltype(PH1)>(PH1)); };
}

int EventClientLogin::Push(lua_State* L) const {
    lua_pushinteger(L, static_cast<lua_Integer>(1));
    lua_pushinteger(L, static_cast<lua_Integer>(clientId));
    return 2;
}

Event::DescriptorType EventClientLogin::type() const {
    volatile int i = 0;
    return descriptor;
}

EventClientLogin::EventClientLogin(const EventClientLogin &in)  : Event(in) {
    this->clientId = in.clientId;
    this->PushLua = [this](auto && PH1) { return Push(std::forward<decltype(PH1)>(PH1)); };
}
