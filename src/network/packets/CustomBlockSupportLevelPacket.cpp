#include "network/packets/CustomBlockSupportLevelPacket.h"

#include "common/ByteBuffer.h"
#include "network/NetworkClient.h"

#include "Client.h"

namespace D3PP::network {
    void CustomBlockSupportLevelPacket::Read(std::shared_ptr<ByteBuffer> buf) {
        SupportLevel = buf->ReadByte();
    }

    void CustomBlockSupportLevelPacket::Write(std::shared_ptr<ByteBuffer> buf) {
        buf->Write(static_cast<unsigned char>(0x03));
        buf->Write(static_cast<unsigned char>(SupportLevel));
        buf->Purge();
    }

    void CustomBlockSupportLevelPacket::Handle(const std::shared_ptr<IMinecraftClient>& client) {

      //  Client::Login(client->GetId(), client->GetName(), client->GetMppass, client->GetVersion());
    }
}