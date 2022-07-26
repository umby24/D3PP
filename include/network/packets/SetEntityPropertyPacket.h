#ifndef D3PP_SETENTITYPROPERTY_H
#define D3PP_SETENTITYPROPERTY_H
#include <memory>
#include <string>

#include "network/IPacket.h"

namespace D3PP::network {
    class SetEntityPropertyPacket : public IPacket {
    public:
        unsigned char entityId;
        unsigned char propertyType;
        int propertyValue;

        SetEntityPropertyPacket() { };
        int GetLength() override;
        void Read(std::shared_ptr<ByteBuffer> buf) override;
        void Write(std::shared_ptr<ByteBuffer> buf) override;
        void Handle(const std::shared_ptr<IMinecraftClient>& client) override;
    private:
    };
}
#endif
