//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTSYSTEM_H
#define D3PP_EVENTSYSTEM_H
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <lua.hpp>

class Event
{
public:
    virtual~ Event();

    using DescriptorType = const char*;

    /** @returns The descriptor type of this event */
    [[nodiscard]] virtual DescriptorType type() const = 0;
    std::function<int(lua_State*)> PushLua;
};

class Dispatcher
{
public:

    /**
      Describes the slot an observer has to implement to be able to process
      events that are posted.
    */

    using SlotType = std::function< void( const Event& ) >;

    /**
      Subscribes a given slot to a certain event descriptor. The slot will be
      called whenever an event of the specified type has been posted. The order
      in which observers subscribe to certain events matters.
      @param descriptor Event to listen for
      @param slot       Slot to call in case such an event occurs
      @returns Connection handle to identify the observing slot
    */

    static int subscribe( const Event::DescriptorType& descriptor,
                                   SlotType&& slot );

    /**
      Unsubscribes an observer from receiving further events. The observer is
      identified by a connection object.
      @param connection Connection handle identifying the observer
    */

    static void unsubscribe(unsigned int connection );

    /**
      Posts an event to all connected observers. This function has no way of
      knowing whether the event has been processed correctly. It will merely
      pass it on to anyone who might listen.
      @param event Event
    */

    static void post( const Event& event );

    static bool hasDescriptor(const std::string& descriptor);

    static Event::DescriptorType getDescriptor(const std::string& descriptor);
private:

    /** Internal ID to assign to the next function handle */
    static int _nextID;

    /**
      @struct SlotHandle
      @brief  Auxiliary data structure for managing slots
    */

    struct SlotHandle
    {
        int id;
        SlotType slot;
    };

    /**
      Maps event descriptors to a vector of slot handles. Each slot handle has a
      unique ID and a corresponding slot that is to be called when an event
      occurs.
    */

    static std::map< Event::DescriptorType, std::vector<SlotHandle> > _observers;
};
#endif //D3PP_EVENTSYSTEM_H
