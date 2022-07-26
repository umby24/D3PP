#include "network/packets/SetMapEnvUrlPacket.h"

#include "common/ByteBuffer.h"
#include "network/NetworkClient.h"

int D3PP::network::SetMapEnvUrlPacket::GetLength()
{
	return 65;
}

void D3PP::network::SetMapEnvUrlPacket::Read(std::shared_ptr<ByteBuffer> buf)
{
}

void D3PP::network::SetMapEnvUrlPacket::Write(std::shared_ptr<ByteBuffer> buf)
{
	buf->Write(static_cast<unsigned char>(0x28));
	buf->Write(textureUrl);
	buf->Purge();
}

void D3PP::network::SetMapEnvUrlPacket::Handle(const std::shared_ptr<IMinecraftClient>& client)
{
}
