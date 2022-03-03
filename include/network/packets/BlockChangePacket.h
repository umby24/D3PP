//
// Created by Wande on 1/30/2022.
//

#ifndef D3PP_BLOCKCHANGEPACKET_H
#define D3PP_BLOCKCHANGEPACKET_H

#include <memory>
#include "network/IPacket.h"
#include "../../common/Vectors.h"

namespace D3PP::network {
    class BlockChangePacket : IPacket {
    public:
        int GetLength() override { return 8; }
        void Read(std::shared_ptr<ByteBuffer> buf) override;
        void Write(std::shared_ptr<ByteBuffer> buf) override;
        void Handle(const std::shared_ptr<IMinecraftClient>& client) override;
    private:
        Common::Vector3S m_blockLocation;
        char m_createMode;
        unsigned char m_blockType;
    };
}
#endif //D3PP_BLOCKCHANGEPACKET_H
