#include "network/packets/SetMapEnvPropertyPacket.h"

#include "common/ByteBuffer.h"
#include "network/NetworkClient.h"

int D3PP::network::SetMapEnvPropertyPacket::GetLength()
{
	return 6;
}

void D3PP::network::SetMapEnvPropertyPacket::Read(std::shared_ptr<ByteBuffer> buf)
{
}

void D3PP::network::SetMapEnvPropertyPacket::Write(std::shared_ptr<ByteBuffer> buf)
{
	buf->Write(static_cast<unsigned char>(0x29));
	buf->Write(propertyType);
	buf->Write(propertyValue);
	buf->Purge();
}

void D3PP::network::SetMapEnvPropertyPacket::Handle(const std::shared_ptr<IMinecraftClient>& client)
{
}
