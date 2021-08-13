//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTMAPACTIONFILL_H
#define D3PP_EVENTMAPACTIONFILL_H
#include "EventSystem.h"
class EventMapActionFill : public Event {
public:
    EventMapActionFill();
    static constexpr DescriptorType descriptor = "Map_Action_Fill";
    virtual DescriptorType type() const;

    int actionId;
    int mapId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTMAPACTIONFILL_H
