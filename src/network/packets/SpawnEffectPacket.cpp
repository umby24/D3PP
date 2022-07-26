//
// Created by Wande on 6/30/2022.
//

#include "network/packets/SpawnEffectPacket.h"
#include "common/ByteBuffer.h"
#include "network/NetworkClient.h"

void D3PP::network::SpawnEffectPacket::Write(std::shared_ptr<ByteBuffer> buf) {
    buf->Write(static_cast<unsigned char>(0x31));
    buf->Write(static_cast<unsigned char>(effectId));
    buf->Write(positionX*32);
    buf->Write(positionZ*32);
    buf->Write(positionY*32);
    buf->Write(originX*32);
    buf->Write(originZ*32);
    buf->Write(originY*32);


    buf->Purge();
}

void D3PP::network::SpawnEffectPacket::Read(std::shared_ptr<ByteBuffer> buf) {

}

void D3PP::network::SpawnEffectPacket::Handle(const std::shared_ptr<IMinecraftClient> &client) {

}
