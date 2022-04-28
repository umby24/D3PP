#include "events/EventChatMap.h"

constexpr EventChatMap::DescriptorType EventChatMap::descriptor;

EventChatMap::EventChatMap() {
    this->PushLua = std::bind(&EventChatMap::Push, this, std::placeholders::_1);
}

int EventChatMap::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, entityId);
    lua_pushstring(L, message.c_str());
    return 3;
}

Event::DescriptorType EventChatMap::type() const {
    return descriptor;
}