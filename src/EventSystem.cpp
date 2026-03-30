//
// Created by unknown on 8/9/21.
//
#include "EventSystem.h"

#include <ranges>


std::map< Event::DescriptorType, std::vector<Dispatcher::SlotHandle> > Dispatcher::_observers;
unsigned int Dispatcher::_nextID = 0;

Event::~Event()
= default;

unsigned int Dispatcher::subscribe( const Event::DescriptorType& descriptor,
                                    SlotType&& slot )
{
    const auto id           = _nextID;
    const SlotHandle handle = { id, slot };

    _observers[ descriptor ].push_back( handle );
    ++_nextID;

    return id;
}

void Dispatcher::unsubscribe(unsigned int connection )
{
    for(auto &val: _observers | std::views::values)
    {
        auto&& handles = val;

        handles.erase( std::ranges::remove_if(handles,
                                              [&] (const SlotHandle& handle )
                                              {
                                                  return handle.id == connection;
                                              } ).begin(),
                       handles.end() );
    }
}

bool Dispatcher::hasDescriptor(const std::string& item) {
    bool found = false;
    for (const auto &key: _observers | std::views::keys) {
        if (key == item) {
            found = true;
            break;
        }
    }
    return found;
}

void Dispatcher::post( Event& event )
{
    const Event::DescriptorType type = event.type();

    if (!_observers.contains(type))
        return;

    for(auto&& observers = _observers.at( type ); auto&[id, eventHandler] : observers )
        eventHandler( event );
}

Event::DescriptorType Dispatcher::getDescriptor(const std::string& descriptor) {
    for (const auto &eventName: _observers | std::views::keys)
        if (eventName == descriptor)
            return eventName;

    return nullptr;
}
