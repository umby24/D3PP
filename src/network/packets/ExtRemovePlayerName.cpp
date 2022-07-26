//
// Created by Wande on 6/30/2022.
//

#include "network/packets/ExtRemovePlayerName.h"
#include "common/ByteBuffer.h"

using namespace D3PP::network;

int ExtRemovePlayerName::GetLength() {
    return 3;
}

void ExtRemovePlayerName::Read(std::shared_ptr<ByteBuffer> buf) {
    m_nameId = buf->ReadShort();
}

void ExtRemovePlayerName::Write(std::shared_ptr<ByteBuffer> buf) {
    buf->Write(static_cast<unsigned char>(24));
    buf->Write(m_nameId);
    buf->Purge();
}

void ExtRemovePlayerName::Handle(const std::shared_ptr<IMinecraftClient> &client) {
    // -- Clientbound only
}