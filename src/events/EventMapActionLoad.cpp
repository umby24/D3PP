#include "events/EventMapActionLoad.h"

constexpr EventMapActionLoad::DescriptorType EventMapActionLoad::descriptor;

EventMapActionLoad::EventMapActionLoad() {
    this->PushLua = std::bind(&EventMapActionLoad::Push, this, std::placeholders::_1);
}

int EventMapActionLoad::Push(lua_State* L) {

}

Event::DescriptorType EventMapActionLoad::type() const {
    return descriptor;
}