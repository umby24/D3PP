//
// Created by Wande on 1/30/2022.
//

#ifndef D3PP_CHATPACKET_H
#define D3PP_CHATPACKET_H
#include <memory>
#include <string>

#include "network/IPacket.h"

namespace D3PP::network {
    class ChatPacket : public IPacket {
    public:
        ChatPacket(const char &playerId, const std::string &message) { m_playerId = playerId; m_message = message; };
        int GetLength() override;
        void Read(std::shared_ptr<ByteBuffer> buf) override;
        void Write(std::shared_ptr<ByteBuffer> buf) override;
        void Handle(const std::shared_ptr<IMinecraftClient>& client) override;
    private:
        char m_playerId;
        std::string m_message;
    };
}
#endif //D3PP_HANDSHAKEPACKET_H
