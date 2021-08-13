//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTMAPADD_H
#define D3PP_EVENTMAPADD_H
class EventMapAdd : public Event {
public:
    EventMapAdd();
    static constexpr DescriptorType descriptor = "Map_Add";
    virtual DescriptorType type() const;

    int mapId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTMAPADD_H
