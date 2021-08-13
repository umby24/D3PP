#include "events/EventMapBlockChangePlayer.h"

constexpr EventMapBlockChangePlayer::DescriptorType EventMapBlockChangePlayer::descriptor;

EventMapBlockChangePlayer::EventMapBlockChangePlayer() {
    this->PushLua = std::bind(&EventMapBlockChangePlayer::Push, this, std::placeholders::_1);
}

int EventMapBlockChangePlayer::Push(lua_State* L) {

}

Event::DescriptorType EventMapBlockChangePlayer::type() const {
    return descriptor;
}