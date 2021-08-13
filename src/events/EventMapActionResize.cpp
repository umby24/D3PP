#include "events/EventMapActionResize.h"

constexpr EventMapActionResize::DescriptorType EventMapActionResize::descriptor;

EventMapActionResize::EventMapActionResize() {
    this->PushLua = std::bind(&EventMapActionResize::Push, this, std::placeholders::_1);
}

int EventMapActionResize::Push(lua_State* L) {

}

Event::DescriptorType EventMapActionResize::type() const {
    return descriptor;
}