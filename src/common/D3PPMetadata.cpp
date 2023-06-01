//
// Created by Wande on 8/11/2022.
//

#include "common/D3PPMetadata.h"
#define st_c static_cast<unsigned char>

Nbt::TagCompound D3PP::Common::D3PPMetadata::Read(Nbt::TagCompound metadata) {
    if (!metadata.data.contains("D3PP"))
        return metadata;

    Nbt::TagCompound cpeBase = std::get<Nbt::TagCompound>(metadata["D3PP"]);
    if (cpeBase.data.contains("Ranks")) {
        auto ranksBase = std::get<Nbt::TagCompound>(cpeBase["Ranks"]);
        JoinRank = std::get<Nbt::TagShort>(ranksBase["Join"]);
        ShowRank = std::get<Nbt::TagShort>(ranksBase["Show"]);
        BuildRank = std::get<Nbt::TagShort>(ranksBase["Build"]);
    }
    if (cpeBase.data.contains("Portals")) {
        portals.clear();

        auto portalsBase = std::get<Nbt::TagCompound>(cpeBase["Portals"]);
        for(auto const &t : portalsBase.data) {
            world::Teleporter newTp;
            auto comp = std::get<Nbt::TagCompound>(t.second);
            newTp.Name = t.first;
            newTp.DestinationMap = std::get<Nbt::TagString>(comp["DestinationMap"]);
        }
    }
    if (cpeBase.data.contains("Particles")) {
        particles.clear();
        auto particlesBase = std::get<Nbt::TagList>(cpeBase["Particles"]);
        bool isCompound = std::holds_alternative<std::vector<Nbt::TagCompound>>(particlesBase.base);
        if (isCompound) {
            auto partiList = std::get<std::vector<Nbt::TagCompound>>(particlesBase.base);

            for (Nbt::TagCompound i : partiList) {
                world::CustomParticle newP;
                newP.effectId = st_c(std::get<Nbt::TagByte>(i["effectId"]));
                newP.U1 = st_c(std::get<Nbt::TagByte>(i["U1"]));
                newP.V1 = st_c(std::get<Nbt::TagByte>(i["V1"]));
                newP.U2 = st_c(std::get<Nbt::TagByte>(i["U2"]));
                newP.V2 = st_c(std::get<Nbt::TagByte>(i["V2"]));
                newP.redTint = st_c(std::get<Nbt::TagByte>(i["redTint"]));
                newP.greenTint = st_c(std::get<Nbt::TagByte>(i["greenTint"]));
                newP.blueTint = st_c(std::get<Nbt::TagByte>(i["blueTint"]));
                newP.frameCount = st_c(std::get<Nbt::TagByte>(i["frameCount"]));
                newP.particleCount = st_c(std::get<Nbt::TagByte>(i["particleCount"]));
                newP.size = st_c(std::get<Nbt::TagByte>(i["size"]));
                newP.sizeVariation = std::get<Nbt::TagInt>(i["sizeVariation"]);
                newP.spread = std::get<Nbt::TagShort>(i["spread"]);
                newP.speed = std::get<Nbt::TagInt>(i["speed"]);
                newP.gravity = std::get<Nbt::TagInt>(i["gravity"]);
                newP.baseLifetime = std::get<Nbt::TagInt>(i["baseLifetime"]);
                newP.lifetimeVariation = std::get<Nbt::TagInt>(i["lifetimeVariation"]);
                newP.collideFlags = st_c(std::get<Nbt::TagByte>(i["collideFlags"]));
                newP.fullBright = std::get<Nbt::TagByte>(i["fullBright"]);

                particles.push_back(newP);
            }
        }
    }

    metadata.data.erase("D3PP");
    return metadata;
}

Nbt::TagCompound D3PP::Common::D3PPMetadata::Write() {
    Nbt::TagCompound base;
    base.name = "D3PP";

    Nbt::TagCompound ranks;
    ranks.data.insert( {"Join", { Nbt::TagShort { JoinRank }}});
    ranks.data.insert({"Show", { Nbt::TagShort { ShowRank }}});
    ranks.data.insert({"Build", { Nbt::TagShort{BuildRank }}});

    base.data.insert({ "Ranks", { ranks }});

    Nbt::TagCompound ports;

    for(auto const& p : portals) {
        auto startBlockCoords = p.OriginStart.GetAsBlockCoords();
        auto endBlockCoords = p.OriginEnd.GetAsBlockCoords();
        auto destBlockCoords = p.Destination.GetAsBlockCoords();

        Nbt::TagCompound thisPortal;
        thisPortal.data.insert({ "Id", { Nbt::TagString { p.Name}}});
        thisPortal.data.insert( {"startX", { Nbt::TagShort { startBlockCoords.X }}});
        thisPortal.data.insert( {"startY", { Nbt::TagShort { startBlockCoords.Y }}});
        thisPortal.data.insert( {"startZ", { Nbt::TagShort { startBlockCoords.Z }}});
        thisPortal.data.insert( {"endX", { Nbt::TagShort { endBlockCoords.X }}});
        thisPortal.data.insert( {"endY", { Nbt::TagShort { endBlockCoords.Y }}});
        thisPortal.data.insert( {"endZ", { Nbt::TagShort { endBlockCoords.Z }}});
        thisPortal.data.insert({ "Destination", { Nbt::TagString{ p.DestinationMap }}});
        thisPortal.data.insert({"DestX", { Nbt::TagShort { destBlockCoords.X }}});
        thisPortal.data.insert({"DestY", { Nbt::TagShort { destBlockCoords.Y }}});
        thisPortal.data.insert({"DestZ", { Nbt::TagShort { destBlockCoords.Z }}});

        ports.data.insert({p.Name, { thisPortal }});
    }
    base.data.insert({ "Portals", { ports }});

    Nbt::TagList parts;
    auto partList = std::vector<Nbt::TagCompound>();

    for(auto const& p : particles) {
        Nbt::TagCompound thisParticle;
        thisParticle.data.insert({ "effectId", { Nbt::TagByte { static_cast<Nbt::TagByte>(p.effectId) }}});
        thisParticle.data.insert({ "U1", { Nbt::TagByte { static_cast<Nbt::TagByte>(p.U1) }}});
        thisParticle.data.insert({ "V1", { Nbt::TagByte { static_cast<Nbt::TagByte>(p.V1) }}});
        thisParticle.data.insert({ "U2", { Nbt::TagByte { static_cast<Nbt::TagByte>(p.U2) }}});
        thisParticle.data.insert({ "V2", { Nbt::TagByte { static_cast<Nbt::TagByte>(p.V2) }}});
        thisParticle.data.insert({ "redTint", { Nbt::TagByte { static_cast<Nbt::TagByte>(p.redTint) }}});
        thisParticle.data.insert({ "greenTint", { Nbt::TagByte { static_cast<Nbt::TagByte>(p.greenTint) }}});
        thisParticle.data.insert({ "blueTint", { Nbt::TagByte { static_cast<Nbt::TagByte>(p.blueTint) }}});
        thisParticle.data.insert({ "frameCount", { Nbt::TagByte { static_cast<Nbt::TagByte>(p.frameCount) }}});
        thisParticle.data.insert({ "particleCount", { Nbt::TagByte { static_cast<Nbt::TagByte>(p.particleCount) }}});
        thisParticle.data.insert({ "size", { Nbt::TagByte { static_cast<Nbt::TagByte>(p.size) }}});
        thisParticle.data.insert({ "sizeVariation", { Nbt::TagInt { p.sizeVariation }}});
        thisParticle.data.insert({ "spread", { Nbt::TagShort { static_cast<Nbt::TagShort>(p.spread) }}});
        thisParticle.data.insert({ "speed", { Nbt::TagInt { p.speed }}});
        thisParticle.data.insert({ "gravity", { Nbt::TagInt { p.gravity }}});
        thisParticle.data.insert({ "baseLifetime", { Nbt::TagInt { p.baseLifetime }}});
        thisParticle.data.insert({ "lifetimeVariation", { Nbt::TagInt { p.lifetimeVariation }}});
        thisParticle.data.insert({ "collideFlags", { Nbt::TagByte { static_cast<Nbt::TagByte>(p.collideFlags) }}});
        thisParticle.data.insert({ "fullBright", { Nbt::TagByte { static_cast<Nbt::TagByte>(p.fullBright) }}});

        partList.push_back(thisParticle);
    }
    parts.base = partList;
    base.data.insert({"Particles", { parts }});

    return base;
}
