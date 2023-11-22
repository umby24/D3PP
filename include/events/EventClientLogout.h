//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTCLIENTLOGOUT_H
#define D3PP_EVENTCLIENTLOGOUT_H
#include "EventSystem.h"
class EventClientLogout : public Event {
public:
    EventClientLogout();
    EventClientLogout(EventClientLogout const &);
    EventClientLogout* clone() const override { return new EventClientLogout(*this); }
    static constexpr DescriptorType descriptor = "Client_Logout";
    virtual DescriptorType type() const;

    int clientId;
    
    int Push(lua_State *L) const;
};
#endif //D3PP_EVENTCLIENTLOGOUT_H
