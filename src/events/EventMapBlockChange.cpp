#include "events/EventMapBlockChange.h"

EventMapBlockChange::EventMapBlockChange() {
    this->PushLua = std::bind(&EventMapBlockChange::Push, this, std::placeholders::_1);
}

int EventMapBlockChange::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, playerNumber);
    lua_pushinteger(L, mapId);
    lua_pushinteger(L, X);
    lua_pushinteger(L, Y);
    lua_pushinteger(L, Z);
    lua_pushinteger(L, bType);
    lua_pushboolean(L, undo);
    lua_pushboolean(L, physic);
    lua_pushboolean(L, send);
    lua_pushinteger(L, priority);
    
    return 11;
}

Event::DescriptorType EventMapBlockChange::type() const {
    return descriptor;
}

EventMapBlockChange::EventMapBlockChange(const EventMapBlockChange &in) : Event(in) {
    this->playerNumber = in.playerNumber;
    this->mapId = in.mapId;
    this->X = in.X;
    this->Y = in.Y;
    this->Z = in.Z;
    this->bType = in.bType;
    this->undo = in.undo;
    this->physic = in.physic;
    this->send = in.send;
    this->priority = in.priority;

    this->PushLua = std::bind(&EventMapBlockChange::Push, this, std::placeholders::_1);
}
