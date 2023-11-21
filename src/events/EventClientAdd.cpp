#include "events/EventClientAdd.h"

constexpr EventClientAdd::DescriptorType EventClientAdd::descriptor;

EventClientAdd::EventClientAdd() {
    this->PushLua = std::bind(&EventClientAdd::Push, this, std::placeholders::_1);
}

int EventClientAdd::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, clientId);
    return 2;
}

Event::DescriptorType EventClientAdd::type() const {
    return descriptor;
}

EventClientAdd::EventClientAdd(const EventClientAdd &in) {
    this->clientId = in.clientId;
}
