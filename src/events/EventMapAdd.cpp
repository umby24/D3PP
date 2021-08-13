#include "events/EventMapAdd.h"

constexpr EventMapAdd::DescriptorType EventMapAdd::descriptor;

EventMapAdd::EventMapAdd() {
    this->PushLua = std::bind(&EventMapAdd::Push, this, std::placeholders::_1);
}

int EventMapAdd::Push(lua_State* L) {

}

Event::DescriptorType EventMapAdd::type() const {
    return descriptor;
}