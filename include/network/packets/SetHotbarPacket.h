#ifndef D3PP_SETHOTBAR_H
#define D3PP_SETHOTBAR_H

#include <memory>
#include <string>

#include "network/IPacket.h"

namespace D3PP::network {
    class SetHotbarPacket : public IPacket {
    public:
        short blockId;
        unsigned char hotbarIndex;
        bool extended;
        SetHotbarPacket(bool isExtended) { extended = isExtended;  };
        int GetLength() override;
        void Read(std::shared_ptr<ByteBuffer> buf) override;
        void Write(std::shared_ptr<ByteBuffer> buf) override;
        void Handle(const std::shared_ptr<IMinecraftClient>& client) override;
    private:
    };
}

#endif
