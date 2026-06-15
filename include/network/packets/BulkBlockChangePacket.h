//
// Created by unknown on 6/15/26.
//

#ifndef D3PP_BULKBLOCKCHANGEPACKET_H
#define D3PP_BULKBLOCKCHANGEPACKET_H
#include <vector>

#include "network/IPacket.h"

namespace D3PP::network {
    class BulkBlockChangePacket : public IPacket {
        public:
        BulkBlockChangePacket(unsigned char count, std::vector <unsigned char> indices, std::vector <unsigned char> blocks);
        int GetLength() override { return 8; }
        void Read(std::shared_ptr<ByteBuffer> buf) override;
        void Write(std::shared_ptr<ByteBuffer> buf) override;
        void Handle(const std::shared_ptr<IMinecraftClient>& client) override;
        private:
        unsigned char m_count;
        std::vector<unsigned char> m_indices;
        std::vector<unsigned char> m_blocks;
    };
}
#endif //D3PP_BULKBLOCKCHANGEPACKET_H
