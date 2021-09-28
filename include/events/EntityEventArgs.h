//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_ENTITYEVENTARGS_H
#define D3PP_ENTITYEVENTARGS_H
#define ENTITY_EVENT_MOVED "ENTITY_MOVED"
#define ENTITY_EVENT_SPAWN "ENTITY_SPAWN"
#define ENTITY_EVENT_DESPAWN "ENTITY_DESPAWN"

#include "EventSystem.h"


class EntityEventArgs : public Event {
public:
    EntityEventArgs(DescriptorType inDescript);
    DescriptorType descriptor = "ENTITY_MOVED";
    [[nodiscard]] DescriptorType type() const override;
    
    int entityId;

    int Push(lua_State *L);
};
#endif //D3PP_EVENTENTITYPOSITIONSET_H
