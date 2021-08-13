#include "events/EventMapActionDelete.h"

constexpr EventMapActionDelete::DescriptorType EventMapActionDelete::descriptor;

EventMapActionDelete::EventMapActionDelete() {
    this->PushLua = std::bind(&EventMapActionDelete::Push, this, std::placeholders::_1);
}

int EventMapActionDelete::Push(lua_State* L) {

}

Event::DescriptorType EventMapActionDelete::type() const {
    return descriptor;
}