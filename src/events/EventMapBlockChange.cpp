#include "events/EventMapBlockChange.h"

constexpr EventMapBlockChange::DescriptorType EventMapBlockChange::descriptor;

EventMapBlockChange::EventMapBlockChange() {
    this->PushLua = std::bind(&EventMapBlockChange::Push, this, std::placeholders::_1);
}

int EventMapBlockChange::Push(lua_State* L) {

}

Event::DescriptorType EventMapBlockChange::type() const {
    return descriptor;
}