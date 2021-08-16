#include "events/EventChatAll.h"

constexpr EventChatAll::DescriptorType EventChatAll::descriptor;

EventChatAll::EventChatAll() {
    this->PushLua = std::bind(&EventChatAll::Push, this, std::placeholders::_1);
}

int EventChatAll::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, entityId);
    lua_pushstring(L, message.c_str());
    return 3;
}

Event::DescriptorType EventChatAll::type() const {
    return descriptor;
}