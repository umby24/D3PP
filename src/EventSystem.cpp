//
// Created by unknown on 8/9/21.
//
#include "EventSystem.h"

std::map< Event::DescriptorType, std::vector<Dispatcher::SlotHandle> > Dispatcher::_observers;
unsigned int Dispatcher::_nextID = 0;

Event::~Event()
{
}

unsigned int Dispatcher::subscribe( const Event::DescriptorType& descriptor,
                                    SlotType&& slot )
{
    auto id           = _nextID;
    SlotHandle handle = { id, slot };

    _observers[ descriptor ].push_back( handle );
    ++_nextID;

    return id;
}

void Dispatcher::unsubscribe(unsigned int connection )
{
    for( auto&& pair : _observers )
    {
        auto&& handles = pair.second;

        handles.erase( std::remove_if( handles.begin(), handles.end(),
                                       [&] ( SlotHandle& handle )
                                       {
                                           return handle.id == connection;
                                       } ),
                       handles.end() );
    }
}

bool Dispatcher::hasdescriptor(std::string item) {
    bool found = false;
    for (auto&& pair : _observers) {
        if (pair.first == item) {
            found = true;
            break;
        }
    }
    return found;
}

void Dispatcher::post( const Event& event )
{
    Event::DescriptorType type = event.type();

    if (_observers.find(type) == _observers.end())
        return;

    auto&& observers = _observers.at( type );

    for( auto&& observer : observers )
        observer.slot( event );
}

Event::DescriptorType Dispatcher::getDescriptor(std::string descriptor) {
    bool found = false;
    for (auto&& pair : _observers) {
        if (pair.first == descriptor) {
            found = true;
            return pair.first;
        }
    }
    return nullptr;
}
