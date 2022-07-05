//
// Created by Wande on 6/30/2022.
//

#include "network/packets/DefineEffectPacket.h"
#include "common/ByteBuffer.h"
#include "network/NetworkClient.h"

void D3PP::network::DefineEffectPacket::Write(std::shared_ptr<ByteBuffer> buf) {
    buf->Write(static_cast<unsigned char>(0x30));
    buf->Write(static_cast<unsigned char>(effectId));
    buf->Write(U1);
    buf->Write(V1);
    buf->Write(U2);
    buf->Write(V2);
    buf->Write(redTint);
    buf->Write(greenTint);
    buf->Write(blueTint);
    buf->Write(frameCount);
    buf->Write(particleCount);
    buf->Write(size);
    buf->Write(sizeVariation);
    buf->Write(static_cast<short>(spread));
    buf->Write(speed);
    buf->Write(gravity);
    buf->Write(baseLifetime);
    buf->Write(lifetimeVariation);
    buf->Write(collideFlags);
    buf->Write(static_cast<unsigned char>(fullBright));

    buf->Purge();
}

void D3PP::network::DefineEffectPacket::Read(std::shared_ptr<ByteBuffer> buf) {

}

void D3PP::network::DefineEffectPacket::Handle(const std::shared_ptr<IMinecraftClient> &client) {

}
