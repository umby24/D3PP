//
// Created by Wande on 7/1/2022.
//

#ifndef D3PP_CUSTOMPARTICLE_H
#define D3PP_CUSTOMPARTICLE_H
/*
 * A particle should be related to a map, as they will be different per texture pack.
 * Each map will be able to define up to 255 different particles
 *
 */
class CustomParticle {
public:
    unsigned char effectId;
    unsigned char U1;
    unsigned char V1;
    unsigned char U2;
    unsigned char V2;
    unsigned char redTint;
    unsigned char greenTint;
    unsigned char blueTint;
    unsigned char frameCount;
    unsigned char particleCount;
    unsigned char size;
    int sizeVariation;
    unsigned short spread;
    int speed;
    int gravity;
    int baseLifetime;
    int lifetimeVariation;
    unsigned char collideFlags;
    char fullBright;

    static void Add(CustomParticle p);
    static void Remove(unsigned char effectId);
    static void Save();
    static void Load();
};
#endif //D3PP_CUSTOMPARTICLE_H
