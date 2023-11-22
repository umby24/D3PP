//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTENTITYDIE_H
#define D3PP_EVENTENTITYDIE_H
#include "EventSystem.h"
class EventEntityDie : public Event {
public:
    EventEntityDie();
    EventEntityDie(EventEntityDie const &);
    EventEntityDie* clone() const override { return new EventEntityDie(*this); }
    static constexpr DescriptorType descriptor = "Entity_Die";
    virtual DescriptorType type() const;

    int entityId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTENTITYDIE_H
