#include "events/EventClientDelete.h"

constexpr EventClientDelete::DescriptorType EventClientDelete::descriptor;

EventClientDelete::EventClientDelete() {
    this->PushLua = std::bind(&EventClientDelete::Push, this, std::placeholders::_1);
}

int EventClientDelete::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, clientId);
    return 2;
}

Event::DescriptorType EventClientDelete::type() const {
    return descriptor;
}