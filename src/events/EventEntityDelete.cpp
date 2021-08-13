#include "events/EventEntityDelete.h"

constexpr EventEntityDelete::DescriptorType EventEntityDelete::descriptor;

EventEntityDelete::EventEntityDelete() {
    this->PushLua = std::bind(&EventEntityDelete::Push, this, std::placeholders::_1);
}

int EventEntityDelete::Push(lua_State* L) {

}

Event::DescriptorType EventEntityDelete::type() const {
    return descriptor;
}