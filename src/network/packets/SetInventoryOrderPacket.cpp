#include "network/packets/SetInventoryOrderPacket.h"

#include "common/ByteBuffer.h"
#include "network/NetworkClient.h"

int D3PP::network::SetInventoryOrderPacket::GetLength()
{
	return 3;
}

void D3PP::network::SetInventoryOrderPacket::Read(std::shared_ptr<ByteBuffer> buf)
{
}

void D3PP::network::SetInventoryOrderPacket::Write(std::shared_ptr<ByteBuffer> buf)
{
	buf->Write(static_cast<unsigned char>(0x2C));
	buf->Write(order);
	buf->Write(blockId);
	buf->Purge();
}

void D3PP::network::SetInventoryOrderPacket::Handle(const std::shared_ptr<IMinecraftClient>& client)
{
}
