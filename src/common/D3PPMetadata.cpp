//
// Created by Wande on 8/11/2022.
//

#include "common/D3PPMetadata.h"

Nbt::TagCompound D3PP::Common::D3PPMetadata::Read(Nbt::TagCompound metadata) {
    return Nbt::TagCompound();
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

    return base;
}
