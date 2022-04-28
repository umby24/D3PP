//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTENTITYDELETE_H
#define D3PP_EVENTENTITYDELETE_H
#include "EventSystem.h"
class EventEntityDelete: public Event {
public:
    EventEntityDelete();
    static constexpr DescriptorType descriptor = "Entity_Delete";
    virtual DescriptorType type() const;

    int entityId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTENTITYDELETE_H
