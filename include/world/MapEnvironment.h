//
// Created by Wande on 1/21/2022.
//

#ifndef D3PP_MAPENVIRONMENT_H
#define D3PP_MAPENVIRONMENT_H
#include <string>
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
        // -- EnvMapAspect
        int cloudHeight, maxFogDistance, cloudSpeed, weatherSpeed, weatherFade, expoFog, mapSideOffset;

        MapEnvironment() {
            SkyColor = -1;
            CloudColor = -1;
            FogColor = -1;
            Alight = -1;
            DLight = -1;
            CanFly = true;
            CanClip = true;
            CanSpeed = true;
            CanRespawn = true;
            CanThirdPerson = true;
            CanSetWeather = true;
            JumpHeight = -1;
            SideLevel = -1;
            SideBlock = 7;
            EdgeBlock = 8;
            cloudHeight = 128;
            maxFogDistance = 0;
            cloudSpeed = 256;
            weatherSpeed = 256;
            weatherFade = 128;
            expoFog = 0;
            mapSideOffset = -2;
        }
    };
}
#endif //D3PP_MAPENVIRONMENT_H
