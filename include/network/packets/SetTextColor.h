#ifndef D3PP_SETTEXTCOLOR_H
#define D3PP_SETTEXTCOLOR_H
#include <memory>
#include <string>

#include "network/IPacket.h"

namespace D3PP::network {
    class SetTextColorPacket : public IPacket {
    public:
        unsigned char red;
        unsigned char green;
        unsigned char blue;
        unsigned char alpha;
        unsigned char code;

        SetTextColorPacket() { red = 0; green = 0; blue = 0; code = 'a'; };
        int GetLength() override;
        void Read(std::shared_ptr<ByteBuffer> buf) override;
        void Write(std::shared_ptr<ByteBuffer> buf) override;
        void Handle(const std::shared_ptr<IMinecraftClient>& client) override;
    private:
    };
}
#endif //D3PP_HANDSHAKEPACKET_H
