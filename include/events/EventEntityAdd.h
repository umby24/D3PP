//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTENTITYADD_H
#define D3PP_EVENTENTITYADD_H
class EventEntityAdd : public Event {
public:
    EventEntityAdd();
    static constexpr DescriptorType descriptor = "Entity_Add";
    virtual DescriptorType type() const;

    int entityId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTENTITYADD_H
