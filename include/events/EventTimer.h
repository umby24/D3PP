//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTTIMER_H
#define D3PP_EVENTTIMER_H
#include "EventSystem.h"
class EventTimer : public Event {
public:
    EventTimer();
    static constexpr DescriptorType descriptor = "Timer";
    virtual DescriptorType type() const;

    int Push(lua_State *L);
};
#endif //D3PP_EVENTTIMER_H
