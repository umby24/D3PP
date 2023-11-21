//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTMAPACTIONRESIZE_H
#define D3PP_EVENTMAPACTIONRESIZE_H
#include "EventSystem.h"
class EventMapActionResize : public Event {
public:
    EventMapActionResize();
    EventMapActionResize* clone() const override { return new EventMapActionResize(*this); }
    static constexpr DescriptorType descriptor = "Map_Action_Resize";
    virtual DescriptorType type() const;

    int actionId;
    int mapId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTMAPACTIONRESIZE_H
