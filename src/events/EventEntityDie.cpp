#include "events/EventEntityDie.h"

constexpr EventEntityDie::DescriptorType EventEntityDie::descriptor;

EventEntityDie::EventEntityDie() {
    this->PushLua = std::bind(&EventEntityDie::Push, this, std::placeholders::_1);
}

int EventEntityDie::Push(lua_State* L) {

}

Event::DescriptorType EventEntityDie::type() const {
    return descriptor;
}