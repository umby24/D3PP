#include "events/EventMapActionSave.h"

constexpr EventMapActionSave::DescriptorType EventMapActionSave::descriptor;

EventMapActionSave::EventMapActionSave() {
    this->PushLua = std::bind(&EventMapActionSave::Push, this, std::placeholders::_1);
}

int EventMapActionSave::Push(lua_State* L) {

}

Event::DescriptorType EventMapActionSave::type() const {
    return descriptor;
}