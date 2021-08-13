#include "events/EventEntityPositionSet.h"

constexpr EventEntityPositionSet::DescriptorType EventEntityPositionSet::descriptor;

EventEntityPositionSet::EventEntityPositionSet() {
    this->PushLua = std::bind(&EventEntityPositionSet::Push, this, std::placeholders::_1);
}

int EventEntityPositionSet::Push(lua_State* L) {

}

Event::DescriptorType EventEntityPositionSet::type() const {
    return descriptor;
}