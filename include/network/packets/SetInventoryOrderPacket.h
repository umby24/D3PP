#ifndef D3PP_SETINVENTORYORDER_H
#define D3PP_SETINVENTORYORDER_H

#include <memory>
#include <string>

#include "network/IPacket.h"

namespace D3PP::network {
    class SetInventoryOrderPacket : public IPacket {
    public:
        unsigned char order;
        unsigned char blockId;

        SetInventoryOrderPacket() { };
        int GetLength() override;
        void Read(std::shared_ptr<ByteBuffer> buf) override;
        void Write(std::shared_ptr<ByteBuffer> buf) override;
        void Handle(const std::shared_ptr<IMinecraftClient>& client) override;
    private:
    };
}

#endif
