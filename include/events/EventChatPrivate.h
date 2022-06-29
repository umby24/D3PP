//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTCHATPRIVATE_H
#define D3PP_EVENTCHATPRIVATE_H
#include "EventSystem.h"
#include <string>

class EventChatPrivate : public Event { // -- lol apparently never used in d3..
public:
    EventChatPrivate();
    static constexpr DescriptorType descriptor = "Chat_Private";
    virtual DescriptorType type() const;
    int toEntityId;
    int fromEntityId;
    std::string message;
    int Push(lua_State *L);
};
#endif //D3PP_EVENTCHATPRIVATE_H
