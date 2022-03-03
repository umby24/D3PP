//
// Created by unknown on 10/29/21.
//

#ifndef D3PP_PLAYEREVENTARGS_H
#define D3PP_PLAYEREVENTARGS_H
#define PLAYER_EVENT_CLICKED "PLAYER_CLICKED"

#include <world/Player.h>
#include "EventSystem.h"
#include "common/Vectors.h"

class PlayerEventArgs : public Event {
public:
    PlayerEventArgs(const DescriptorType* inDescript);
    const DescriptorType* descriptor;
    constexpr static DescriptorType clickDescriptor = PLAYER_EVENT_CLICKED;
    [[nodiscard]] DescriptorType type() const override;

    int playerId;

    virtual int Push(lua_State *L) = 0;
};

class PlayerClickEventArgs : public PlayerEventArgs {

    int Push(lua_State *L) override;
public:
    ClickButton button;
    ClickAction action;
    short yaw;
    short pitch;
    char targetEntity;
    D3PP::Common::Vector3S targetBlock;
    ClickTargetBlockFace blockFace;

    PlayerClickEventArgs() : PlayerEventArgs(&clickDescriptor) { this->PushLua = std::bind(&PlayerClickEventArgs::Push, this, std::placeholders::_1); }
};
#endif //D3PP_PLAYEREVENTARGS_H
