//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTMAPBLOCKCHANGECLIENT_H
#define D3PP_EVENTMAPBLOCKCHANGECLIENT_H
#include "EventSystem.h"
class EventMapBlockChangeClient : public Event {
public:
    EventMapBlockChangeClient();
    EventMapBlockChangeClient(EventMapBlockChangeClient const &);
    EventMapBlockChangeClient* clone() const override { return new EventMapBlockChangeClient(*this); }
    static constexpr DescriptorType descriptor = "Map_Block_Change_Client";
    virtual DescriptorType type() const;

    int clientId;
    int mapId;
    unsigned short X;
    unsigned short Y;
    unsigned short Z;
    unsigned char mode;
    unsigned char bType;

    int Push(lua_State *L) const;
};
#endif //D3PP_EVENTMAPBLOCKCHANGECLIENT_H
