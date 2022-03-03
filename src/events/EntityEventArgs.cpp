#include "events/EntityEventArgs.h"

EntityEventArgs::EntityEventArgs(const DescriptorType* inDescript) {
    descriptor = inDescript;
}

int EntityEventArgs::Push(lua_State *L) {
    // nop
    return 0;
}

Event::DescriptorType EntityEventArgs::type() const {
    return *descriptor;
}