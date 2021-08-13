//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTMAPACTIONSAVE_H
#define D3PP_EVENTMAPACTIONSAVE_H
#include "EventSystem.h"
class EventMapActionSave : public Event {
public:
    EventMapActionSave();
    static constexpr DescriptorType descriptor = "Map_Action_Save";
    virtual DescriptorType type() const;

    int actionId;
    int mapId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTMAPACTIONSAVE_H
