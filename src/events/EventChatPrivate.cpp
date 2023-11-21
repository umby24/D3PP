//
// Created by Wande on 6/29/2022.
//

#include "events/EventChatPrivate.h"

constexpr EventChatPrivate::DescriptorType EventChatPrivate::descriptor;

EventChatPrivate::EventChatPrivate() {
    this->PushLua = std::bind(&EventChatPrivate::Push, this, std::placeholders::_1);
}

int EventChatPrivate::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, toEntityId);
    lua_pushinteger(L, fromEntityId);
    lua_pushstring(L, message.c_str());
    return 4;
}

Event::DescriptorType EventChatPrivate::type() const {
    return descriptor;
}

EventChatPrivate::EventChatPrivate(const EventChatPrivate &in) {
    this->toEntityId = in.toEntityId;
    this->fromEntityId = in.fromEntityId;
    this->message = in.message;
}
