//
// Created by Wande on 1/30/2022.
//

#ifndef D3PP_HANDSHAKEPACKET_H
#define D3PP_HANDSHAKEPACKET_H
#include <memory>
#include "network/IPacket.h"

namespace D3PP::network {
    class HandshakePacket : IPacket {
    public:
        int GetLength() override;
        void Read(std::shared_ptr<ByteBuffer> buf) override;
        void Write(std::shared_ptr<ByteBuffer> buf) override;
        void Handle(const std::shared_ptr<IMinecraftClient>& client) override;
    };
}


#endif //D3PP_HANDSHAKEPACKET_H
