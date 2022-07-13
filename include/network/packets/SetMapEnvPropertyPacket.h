#ifndef D3PP_SETMAPENVPROPERTYL_H
#define D3PP_SETMAPENVPROPERTYL_H

#include <memory>
#include <string>

#include "network/IPacket.h"

namespace D3PP::network {
    enum MapEnvProperty {
        SideBlockId = 0,
        EdgeBlockId = 1,
        EdgeHeight,
        CloudHeight,
        FogDistance,
        CloudSpeed,
        WeatherSpeed,
        WeatherFade,
        ExpoFog,
        SideOffset = 9
    };

    class SetMapEnvPropertyPacket : public IPacket {
    public:
        unsigned char propertyType;
        int propertyValue;

        SetMapEnvPropertyPacket() { };
        int GetLength() override;
        void Read(std::shared_ptr<ByteBuffer> buf) override;
        void Write(std::shared_ptr<ByteBuffer> buf) override;
        void Handle(const std::shared_ptr<IMinecraftClient>& client) override;
    private:
    };
}

#endif
