//
// Created by Wande on 1/21/2022.
//

#ifndef D3PP_MAPENVIRONMENT_H
#define D3PP_MAPENVIRONMENT_H
namespace D3PP::world {
    struct MapEnvironment {
        // -- Env Colors
        int SkyColor, CloudColor, FogColor, Alight, DLight;
        // -- Hack Control
        bool CanFly, CanClip, CanSpeed, CanRespawn, CanThirdPerson, CanSetWeather;
        short JumpHeight;
        // -- Env Appearance
        short SideLevel;
        unsigned char SideBlock, EdgeBlock;
        std::string TextureUrl;
    };
}
#endif //D3PP_MAPENVIRONMENT_H
