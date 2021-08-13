#include "events/EventChatMap.h"

constexpr EventChatMap::DescriptorType EventChatMap::descriptor;

EventChatMap::EventChatMap() {
    this->PushLua = std::bind(&EventChatMap::Push, this, std::placeholders::_1);
}

int EventChatMap::Push(lua_State* L) {

}

Event::DescriptorType EventChatMap::type() const {
    return descriptor;
}