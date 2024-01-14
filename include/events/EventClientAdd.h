//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTCLIENTADD_H
#define D3PP_EVENTCLIENTADD_H
#include "EventSystem.h"
class EventClientAdd : public Event {
public:
    EventClientAdd();
    EventClientAdd(EventClientAdd const &);
    EventClientAdd* clone() const override { return new EventClientAdd(*this); }

    static constexpr DescriptorType descriptor = "Client_Add";
    virtual DescriptorType type() const;
    int clientId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTCLIENTADD_H
