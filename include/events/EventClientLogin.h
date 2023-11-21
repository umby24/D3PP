//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTCLIENTLOGIN_H
#define D3PP_EVENTCLIENTLOGIN_H
#include "EventSystem.h"
class EventClientLogin : public Event {
public:
    EventClientLogin();
    EventClientLogin* clone() const override { return new EventClientLogin(*this); }
    static constexpr DescriptorType descriptor = "Client_Login";
    DescriptorType type() const override;// DescriptorType type() const;

    int clientId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTCLIENTLOGIN_H
