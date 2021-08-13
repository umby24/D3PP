#include "events/EventClientLogout.h"

constexpr EventClientLogout::DescriptorType EventClientLogout::descriptor;

EventClientLogout::EventClientLogout() {
    this->PushLua = std::bind(&EventClientLogout::Push, this, std::placeholders::_1);
}

int EventClientLogout::Push(lua_State* L) {

}

Event::DescriptorType EventClientLogout::type() const {
    return descriptor;
}