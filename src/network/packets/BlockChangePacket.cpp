#include "network/packets/BlockChangePacket.h"

#include "common/ByteBuffer.h"
#include "network/NetworkClient.h"
#include "BuildMode.h"

namespace D3PP::network {
    void BlockChangePacket::Read(std::shared_ptr<ByteBuffer> buf) {
        const short x = buf->ReadShort();
        const short z = buf->ReadShort();
        const short y = buf->ReadShort();
        m_blockLocation = Common::Vector3S(x, y, z);
        m_createMode = buf->ReadByte() & 255;
        m_blockType = buf->ReadByte() & 255;
    }

    void BlockChangePacket::Write(std::shared_ptr<ByteBuffer> buf) {
        buf->Write(static_cast<unsigned char>(0x06));
        buf->Write(m_blockLocation.X);
        buf->Write(m_blockLocation.Z);
        buf->Write(m_blockLocation.Y);
        buf->Write(m_createMode);
        buf->Write(m_blockType);
        buf->Purge();
    }

    void BlockChangePacket::Handle(const std::shared_ptr<IMinecraftClient>& client) {
        BuildModeMain* bmm = BuildModeMain::GetInstance();
        bmm->Distribute(client->GetId(), client->GetMapId(), m_blockLocation.X, m_blockLocation.Y, m_blockLocation.Z, (m_createMode > 0), m_blockType);
    }
}