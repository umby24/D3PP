#include "events/EventEntityAdd.h"

constexpr EventEntityAdd::DescriptorType EventEntityAdd::descriptor;

EventEntityAdd::EventEntityAdd() {
    this->PushLua = std::bind(&EventEntityAdd::Push, this, std::placeholders::_1);
}

int EventEntityAdd::Push(lua_State* L) {

}

Event::DescriptorType EventEntityAdd::type() const {
    return descriptor;
}