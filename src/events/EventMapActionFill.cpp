#include "events/EventMapActionFill.h"

constexpr EventMapActionFill::DescriptorType EventMapActionFill::descriptor;

EventMapActionFill::EventMapActionFill() {
    this->PushLua = std::bind(&EventMapActionFill::Push, this, std::placeholders::_1);
}

int EventMapActionFill::Push(lua_State* L) {

}

Event::DescriptorType EventMapActionFill::type() const {
    return descriptor;
}