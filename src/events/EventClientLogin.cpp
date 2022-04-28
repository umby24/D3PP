#include "events/EventClientLogin.h"

EventClientLogin::EventClientLogin() {
    this->PushLua = std::bind(&EventClientLogin::Push, this, std::placeholders::_1);
}

int EventClientLogin::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, clientId);
    return 2;
}

Event::DescriptorType EventClientLogin::type() const {
    volatile int i = 0;
    return descriptor;
}