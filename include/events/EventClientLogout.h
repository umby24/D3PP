//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTCLIENTLOGOUT_H
#define D3PP_EVENTCLIENTLOGOUT_H
class EventClientLogout : public Event {
public:
    EventClientLogout();
    static constexpr DescriptorType descriptor = "Client_Logout";
    virtual DescriptorType type() const;

    int clientId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTCLIENTLOGOUT_H
