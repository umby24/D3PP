#include "events/EntityEventArgs.h"

EntityEventArgs::EntityEventArgs(const DescriptorType* inDescript) {
    descriptor = inDescript;
    this->PushLua = std::bind(&EntityEventArgs::Push, this, std::placeholders::_1);
}

int EntityEventArgs::Push(lua_State *L) {
    lua_pushinteger(L, this->entityId);
    return 1;
}

Event::DescriptorType EntityEventArgs::type() const {
    return *descriptor;
}

EntityEventArgs::EntityEventArgs(const EntityEventArgs &in) {
    this->entityId = in.entityId;
    this->descriptor = in.descriptor;
}
