//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTCLIENTDELETE_H
#define D3PP_EVENTCLIENTDELETE_H
#include "EventSystem.h"
class EventClientDelete : public Event {
public:
    EventClientDelete();
    static constexpr DescriptorType descriptor = "Client_Delete";
    virtual DescriptorType type() const;
    int clientId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTCLIENTDELETE_H
