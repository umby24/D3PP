//
// Created by Wande on 6/30/2022.
//

#ifndef D3PP_SPAWNEFFECTPACKET_H
#define D3PP_SPAWNEFFECTPACKET_H
#include <memory>
#include <string>

#include "network/IPacket.h"

namespace D3PP::network {
    class SpawnEffectPacket : public IPacket {
    public:
        char effectId;
        int positionX;
        int positionY;
        int positionZ;
        int originX;
        int originY;
        int originZ;

        int GetLength() override { return 26; }
        void Read(std::shared_ptr<ByteBuffer> buf) override;
        void Write(std::shared_ptr<ByteBuffer> buf) override;
        void Handle(const std::shared_ptr<IMinecraftClient>& client) override;
    private:
    };
}
#endif //D3PP_SPAWNEFFECTPACKET_H
