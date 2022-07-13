#include "network/packets/SetHotbarPacket.h"

#include "common/ByteBuffer.h"
#include "network/NetworkClient.h"

int D3PP::network::SetHotbarPacket::GetLength()
{
    if (!extended)
        return 3;
    else
        return 4;
}

void D3PP::network::SetHotbarPacket::Read(std::shared_ptr<ByteBuffer> buf)
{

}

void D3PP::network::SetHotbarPacket::Write(std::shared_ptr<ByteBuffer> buf)
{
    buf->Write(static_cast<unsigned char>(0x2D));
    
    if (!extended)
        buf->Write(static_cast<unsigned char>(blockId));
    else
        buf->Write(blockId);

    buf->Write(hotbarIndex);
    buf->Purge();
}

void D3PP::network::SetHotbarPacket::Handle(const std::shared_ptr<IMinecraftClient>& client)
{
}
