//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTENTITYDELETE_H
#define D3PP_EVENTENTITYDELETE_H
#include "EventSystem.h"
class EventEntityDelete: public Event {
public:
    EventEntityDelete();
    EventEntityDelete(EventEntityDelete const &);
    EventEntityDelete* clone() const override { return new EventEntityDelete(*this); }
    static constexpr DescriptorType descriptor = "Entity_Delete";
    virtual DescriptorType type() const;

    int entityId;
    
    int Push(lua_State *L) const;
};
#endif //D3PP_EVENTENTITYDELETE_H
