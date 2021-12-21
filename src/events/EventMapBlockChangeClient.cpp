#include "events/EventMapBlockChangeClient.h"

constexpr EventMapBlockChangeClient::DescriptorType EventMapBlockChangeClient::descriptor;

EventMapBlockChangeClient::EventMapBlockChangeClient() {
    this->PushLua = std::bind(&EventMapBlockChangeClient::Push, this, std::placeholders::_1);
}

int EventMapBlockChangeClient::Push(lua_State* L) {
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