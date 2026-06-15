//
// Created by unknown on 6/15/26.
//

#include "network/packets/BulkBlockChangePacket.h"

#include <utility>

#include "common/ByteBuffer.h"

D3PP::network::BulkBlockChangePacket::BulkBlockChangePacket(unsigned char count, std::vector<unsigned char> indices,
                                                            std::vector<unsigned char> blocks) {
    this->m_count = count;
    this->m_indices = std::move(indices);
    this->m_blocks = std::move(blocks);
}

void D3PP::network::BulkBlockChangePacket::Read(std::shared_ptr<ByteBuffer> buf) {
}

void D3PP::network::BulkBlockChangePacket::Write(std::shared_ptr<ByteBuffer> buf) {
    buf->Write(static_cast<unsigned char>(0x26));
    buf->Write(m_count);
    buf->Write(m_indices, 1024);
    buf->Write(m_blocks, 256);
    buf->Purge();
}

void D3PP::network::BulkBlockChangePacket::Handle(const std::shared_ptr<IMinecraftClient> &client) {
    // -- Noop, Server->Client only.
}
