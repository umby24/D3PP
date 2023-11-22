#include "events/EventMapBlockChangeClient.h"

EventMapBlockChangeClient::EventMapBlockChangeClient() {
    clientId = -1;
    mapId = -1;
    X = 0; Y = 0; Z = 0;
    mode = 0; bType = 0;
    this->PushLua = [this](auto && PH1) { return Push(std::forward<decltype(PH1)>(PH1)); };
}

int EventMapBlockChangeClient::Push(lua_State* L) const {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, clientId);
    lua_pushinteger(L, mapId);
    lua_pushinteger(L, X);
    lua_pushinteger(L, Y);
    lua_pushinteger(L, Z);
    lua_pushinteger(L, mode);
    lua_pushinteger(L, bType);
    return 8;
}

Event::DescriptorType EventMapBlockChangeClient::type() const {
    return descriptor;
}

EventMapBlockChangeClient::EventMapBlockChangeClient(const EventMapBlockChangeClient &in) : Event(in) {
    this->clientId = in.clientId;
    this->mapId = in.mapId;
    this->X = in.X;
    this->Y = in.Y;
    this->Z = in.Z;
    this->mode = in.mode;
    this->bType = in.bType;
    this->PushLua = [this](auto && PH1) { return Push(std::forward<decltype(PH1)>(PH1)); };
}
