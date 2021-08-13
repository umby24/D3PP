//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTMAPACTIONDELETE_H
#define D3PP_EVENTMAPACTIONDELETE_H
class EventMapActionDelete : public Event {
public:
    EventMapActionDelete();
    static constexpr DescriptorType descriptor = "Map_Action_Delete";
    virtual DescriptorType type() const;

    int actionId;
    int mapId;
    
    int Push(lua_State *L);
};
#endif //D3PP_EVENTMAPACTIONDELETE_H
