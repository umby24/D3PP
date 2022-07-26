#ifndef D3PP_SETMAPENVURL_H
#define D3PP_SETMAPENVURL_H

#include <memory>
#include <string>

#include "network/IPacket.h"

namespace D3PP::network {
    class SetMapEnvUrlPacket : public IPacket {
    public:
        std::string textureUrl;

        SetMapEnvUrlPacket() { };
        int GetLength() override;
        void Read(std::shared_ptr<ByteBuffer> buf) override;
        void Write(std::shared_ptr<ByteBuffer> buf) override;
        void Handle(const std::shared_ptr<IMinecraftClient>& client) override;
    private:
    };
}

#endif
