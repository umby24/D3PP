//
// Created by Wande on 6/30/2022.
//

#ifndef D3PP_DEFINEEFFECTPACKET_H
#define D3PP_DEFINEEFFECTPACKET_H
#include <memory>
#include <string>

#include "network/IPacket.h"

namespace D3PP::network {
    class DefineEffectPacket : public IPacket {
    public:
        unsigned char effectId;
        unsigned char U1;
        unsigned char V1;
        unsigned char U2;
        unsigned char V2;
        unsigned char redTint;
        unsigned char greenTint;
        unsigned char blueTint;
        unsigned char frameCount;
        unsigned char particleCount;
        unsigned char size;
        int sizeVariation;
        unsigned short spread;
        int speed;
        int gravity;
        int baseLifetime;
        int lifetimeVariation;
        unsigned char collideFlags;
        char fullBright;

        int GetLength() override { return 36; }
        void Read(std::shared_ptr<ByteBuffer> buf) override;
        void Write(std::shared_ptr<ByteBuffer> buf) override;
        void Handle(const std::shared_ptr<IMinecraftClient>& client) override;
    private:
    };
}
#endif //D3PP_DEFINEEFFECTPACKET_H
