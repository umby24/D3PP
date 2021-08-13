#include "events/EventMapBlockChangeClient.h"

constexpr EventMapBlockChangeClient::DescriptorType EventMapBlockChangeClient::descriptor;

EventMapBlockChangeClient::EventMapBlockChangeClient() {
    this->PushLua = std::bind(&EventMapBlockChangeClient::Push, this, std::placeholders::_1);
}

int EventMapBlockChangeClient::Push(lua_State* L) {

}

Event::DescriptorType EventMapBlockChangeClient::type() const {
    return descriptor;
}