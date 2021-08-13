//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTMAPBLOCKCHANGEPLAYER_H
#define D3PP_EVENTMAPBLOCKCHANGEPLAYER_H
#include "EventSystem.h"

class EventMapBlockChangePlayer : public Event {
public:
    EventMapBlockChangePlayer();
    static constexpr DescriptorType descriptor = "Map_Block_Change_Player";
    virtual DescriptorType type() const;

    int Push(lua_State *L);
};
#endif //D3PP_EVENTMAPBLOCKCHANGEPLAYER_H
