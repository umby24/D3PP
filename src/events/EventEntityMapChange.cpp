#include "events/EventEntityMapChange.h"

constexpr EventEntityMapChange::DescriptorType EventEntityMapChange::descriptor;

EventEntityMapChange::EventEntityMapChange() {
    this->PushLua = std::bind(&EventEntityMapChange::Push, this, std::placeholders::_1);
}

int EventEntityMapChange::Push(lua_State* L) {

}

Event::DescriptorType EventEntityMapChange::type() const {
    return descriptor;
}