#include "events/EventClientAdd.h"

constexpr EventClientAdd::DescriptorType EventClientAdd::descriptor;

EventClientAdd::EventClientAdd() {
    this->PushLua = std::bind(&EventClientAdd::Push, this, std::placeholders::_1);
}

int EventClientAdd::Push(lua_State* L) {

}

Event::DescriptorType EventClientAdd::type() const {
    return descriptor;
}