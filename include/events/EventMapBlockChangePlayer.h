//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTMAPBLOCKCHANGEPLAYER_H
#define D3PP_EVENTMAPBLOCKCHANGEPLAYER_H
#include "EventSystem.h"

class EventMapBlockChangePlayer : public Event {
public:
    EventMapBlockChangePlayer();
    EventMapBlockChangePlayer(EventMapBlockChangePlayer const &);
    EventMapBlockChangePlayer* clone() const override { return new EventMapBlockChangePlayer(*this); }
    static constexpr DescriptorType descriptor = "Map_Block_Change_Player";
    virtual DescriptorType type() const;

    int playerNumber;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTMAPBLOCKCHANGEPLAYER_H
