#include "network/packets/SetTextColor.h"
#include "common/ByteBuffer.h"
#include "network/NetworkClient.h"

int D3PP::network::SetTextColorPacket::GetLength()
{
	return 6;
}

void D3PP::network::SetTextColorPacket::Read(std::shared_ptr<ByteBuffer> buf)
{
	// -- Server -> Client only
}

void D3PP::network::SetTextColorPacket::Write(std::shared_ptr<ByteBuffer> buf)
{
	buf->Write(static_cast<unsigned char>(0x27));
	buf->Write(red);
	buf->Write(green);
	buf->Write(blue);
	buf->Write(alpha);
	buf->Write(code);
	buf->Purge();
}

void D3PP::network::SetTextColorPacket::Handle(const std::shared_ptr<IMinecraftClient>& client)
{
}
