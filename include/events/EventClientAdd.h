//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTCLIENTADD_H
#define D3PP_EVENTCLIENTADD_H
class EventClientAdd : public Event {
public:
    EventClientAdd();
    static constexpr DescriptorType descriptor = "Client_Add";
    virtual DescriptorType type() const;
    int clientId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTCLIENTADD_H
