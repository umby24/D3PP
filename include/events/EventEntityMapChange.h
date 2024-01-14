//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTENTITYMAPCHANGE_H
#define D3PP_EVENTENTITYMAPCHANGE_H
#include "EventSystem.h"
class EventEntityMapChange : public Event {
public:
    EventEntityMapChange();
    EventEntityMapChange(EventEntityMapChange const &);
    EventEntityMapChange* clone() const override { return new EventEntityMapChange(*this); }
    static constexpr DescriptorType descriptor = "Entity_Map_Change";
    virtual DescriptorType type() const;

    int entityId;
    int oldMapId;
    int newMapId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTENTITYMAPCHANGE_H
