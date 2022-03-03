//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_ENTITYEVENTARGS_H
#define D3PP_ENTITYEVENTARGS_H
#define ENTITY_EVENT_MOVED "ENTITY_MOVED"
#define ENTITY_EVENT_SPAWN "Entity_Add"
#define ENTITY_EVENT_DESPAWN "Entity_Delete"

#include "EventSystem.h"


class EntityEventArgs : public Event {
public:
    EntityEventArgs(const DescriptorType* inDescript);
    const DescriptorType* descriptor;
    constexpr static DescriptorType moveDescriptor = ENTITY_EVENT_MOVED;
    constexpr static DescriptorType spawnDescriptor = ENTITY_EVENT_SPAWN;
    [[nodiscard]] DescriptorType type() const override;
    
    int entityId;

    int Push(lua_State *L);
};
#endif //D3PP_EVENTENTITYPOSITIONSET_H
