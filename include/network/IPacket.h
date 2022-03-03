//
// Created by Wande on 1/28/2022.
//

#ifndef D3PP_IPACKET_H
#define D3PP_IPACKET_H
#include <memory>

class ByteBuffer;
class IMinecraftClient;

namespace D3PP::network {
    class IPacket {
    public:
        IPacket()= default;;
        virtual ~IPacket()= default;;
        virtual int GetLength() = 0;
        virtual void Read(std::shared_ptr<ByteBuffer> buf) = 0;
        virtual void Write(std::shared_ptr<ByteBuffer> buf) = 0;
        virtual void Handle(const std::shared_ptr<IMinecraftClient>& client) = 0;
    };
}
#endif //D3PP_IPACKET_H
