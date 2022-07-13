#include "network/packets/SetEntityPropertyPacket.h"

#include "common/ByteBuffer.h"
#include "network/NetworkClient.h"

int D3PP::network::SetEntityPropertyPacket::GetLength()
{
	return 7;
}

void D3PP::network::SetEntityPropertyPacket::Read(std::shared_ptr<ByteBuffer> buf)
{
}

void D3PP::network::SetEntityPropertyPacket::Write(std::shared_ptr<ByteBuffer> buf)
{
	buf->Write(static_cast<unsigned char>(0x2A));
	buf->Write(entityId);
	buf->Write(propertyType);
	buf->Write(propertyValue);
	buf->Purge();
}

void D3PP::network::SetEntityPropertyPacket::Handle(const std::shared_ptr<IMinecraftClient>& client)
{
}
