#include "events/EventMapBlockChangePlayer.h"

constexpr EventMapBlockChangePlayer::DescriptorType EventMapBlockChangePlayer::descriptor;

EventMapBlockChangePlayer::EventMapBlockChangePlayer() {
    this->PushLua = std::bind(&EventMapBlockChangePlayer::Push, this, std::placeholders::_1);
}

int EventMapBlockChangePlayer::Push(lua_State* L) {
    lua_pushinteger(L, 1);
    lua_pushinteger(L, playerNumber);
    
    return 2;//8;
}

Event::DescriptorType EventMapBlockChangePlayer::type() const {
    return descriptor;
}

EventMapBlockChangePlayer::EventMapBlockChangePlayer(const EventMapBlockChangePlayer &in) : Event(in) {
    this->playerNumber = in.playerNumber;
    this->PushLua = std::bind(&EventMapBlockChangePlayer::Push, this, std::placeholders::_1);
}
