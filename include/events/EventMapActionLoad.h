//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTMAPACTIONLOAD_H
#define D3PP_EVENTMAPACTIONLOAD_H
class EventMapActionLoad : public Event {
public:
    EventMapActionLoad();
    static constexpr DescriptorType descriptor = "Map_Action_Load";
    virtual DescriptorType type() const;

    int actionId;
    int mapId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTMAPACTIONLOAD_H
