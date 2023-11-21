//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTCHATMAP_H
#define D3PP_EVENTCHATMAP_H
#include "EventSystem.h"

class EventChatMap : public Event {
public:
    EventChatMap();
    EventChatMap(EventChatMap const &);
    static constexpr DescriptorType descriptor = "Chat_Map";
    virtual DescriptorType type() const;
    EventChatMap* clone() const override { return new EventChatMap(*this); }

    int entityId;
    std::string message;

    int Push(lua_State *L);
};

#endif //D3PP_EVENTCHATMAP_H
