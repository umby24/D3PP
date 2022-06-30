//
// Created by Wande on 6/30/2022.
//

#ifndef D3PP_EXTREMOVEPLAYERNAME_H
#define D3PP_EXTREMOVEPLAYERNAME_H
#include <memory>
#include <string>

#include "network/IPacket.h"


namespace D3PP::network {
    class ExtRemovePlayerName : public IPacket {
    public:
        ExtRemovePlayerName(const short &nameId) { m_nameId = nameId; };
        int GetLength() override;
        void Read(std::shared_ptr<ByteBuffer> buf) override;
        void Write(std::shared_ptr<ByteBuffer> buf) override;
        void Handle(const std::shared_ptr<IMinecraftClient>& client) override;
    private:
        short m_nameId;
    };
}
#endif //D3PP_EXTREMOVEPLAYERNAME_H