//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTCHATALL_H
#define D3PP_EVENTCHATALL_H
#include "EventSystem.h"

class EventChatAll : public Event {
public:
    EventChatAll();
    static constexpr DescriptorType descriptor = "Chat_All";
    virtual DescriptorType type() const;

    int entityId;
    std::string message;

    int Push(lua_State *L);
};
#endif //D3PP_EVENTCHATALL_H
