#include "events/EventClientLogin.h"

constexpr EventClientLogin::DescriptorType EventClientLogin::descriptor;

EventClientLogin::EventClientLogin() {
    this->PushLua = std::bind(&EventClientLogin::Push, this, std::placeholders::_1);
}

int EventClientLogin::Push(lua_State* L) {

}

Event::DescriptorType EventClientLogin::type() const {
    return descriptor;
}