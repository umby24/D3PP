//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTMAPACTIONDELETE_H
#define D3PP_EVENTMAPACTIONDELETE_H
#include "EventSystem.h"
class EventMapActionDelete : public Event {
public:
    EventMapActionDelete();
    EventMapActionDelete* clone() const override { return new EventMapActionDelete(*this); }
    static constexpr DescriptorType descriptor = "Map_Action_Delete";
    virtual DescriptorType type() const;

    int actionId;
    int mapId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTMAPACTIONDELETE_H
