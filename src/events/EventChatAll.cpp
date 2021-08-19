#include "events/EventChatAll.h"

EventChatAll::EventChatAll() {
    this->PushLua = std::bind(&EventChatAll::Push, this, std::placeholders::_1);
}

int EventChatAll::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, entityId);
    lua_pushstring(L, message.c_str());
    return 3;
}

#pragma GCC push_options
#pragma GCC optimize("O0")
Event::DescriptorType EventChatAll::type() const {
    return descriptor;
}
#pragma GCC pop_options