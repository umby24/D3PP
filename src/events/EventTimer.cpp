#include "events/EventTimer.h"

constexpr EventTimer::DescriptorType EventTimer::descriptor;

EventTimer::EventTimer() {
    this->PushLua = std::bind(&EventTimer::Push, this, std::placeholders::_1);
}

int EventTimer::Push(lua_State* L) {

}

Event::DescriptorType EventTimer::type() const {
    return descriptor;
}