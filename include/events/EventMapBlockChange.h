//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTMAPBLOCKCHANGE_H
#define D3PP_EVENTMAPBLOCKCHANGE_H
class EventMapBlockChange : public Event {
public:
    EventMapBlockChange();
    static constexpr DescriptorType descriptor = "Map_Block_Change";
    virtual DescriptorType type() const;

    int playerNumber;
    int mapId;
    short X;
    short Y;
    short Z;
    unsigned char bType;
    bool undo;
    bool physic;
    bool send;
    unsigned char priority;

    int Push(lua_State *L);
};
#endif //D3PP_EVENTMAPBLOCKCHANGE_H
