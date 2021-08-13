#include "events/EventChatAll.h"

constexpr EventChatAll::DescriptorType EventChatAll::descriptor;

EventChatAll::EventChatAll() {
    this->PushLua = std::bind(&EventChatAll::Push, this, std::placeholders::_1);
}

int EventChatAll::Push(lua_State* L) {

}

Event::DescriptorType EventChatAll::type() const {
    return descriptor;
}