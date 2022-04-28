#include "network/packets/ChatPacket.h"

#include "common/ByteBuffer.h"
#include "network/NetworkClient.h"
#include "network/Chat.h"

#include "Utils.h"

namespace D3PP::network {
    void ChatPacket::Read(std::shared_ptr<ByteBuffer> buf) {
        m_playerId = buf->ReadByte();
        m_message = buf->ReadString();
        Utils::TrimString(m_message);
    }

    void ChatPacket::Write(std::shared_ptr<ByteBuffer> buf) {
        buf->Write(static_cast<unsigned char>(13));
        buf->Write(static_cast<unsigned char>(m_playerId));
        if (m_message.size() != 64) Utils::padTo(m_message, 64);
        buf->Write(m_message);
        buf->Purge();
    }

    void ChatPacket::Handle(const std::shared_ptr<IMinecraftClient>& client) {
        std::shared_ptr<NetworkClient> concrete = std::static_pointer_cast<NetworkClient>(client);
        Chat::HandleIncomingChat(concrete, m_message, m_playerId);
    }

    int ChatPacket::GetLength() {
        return 66;
    }
}