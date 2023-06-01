//
// Created by Wande on 8/11/2022.
//

#ifndef D3PP_D3PPMETADATA_H
#define D3PP_D3PPMETADATA_H

#include <world/CustomParticle.h>
#include "files/ClassicWorld.h"
#include "world/Teleporter.h"

namespace D3PP::Common {
    class D3PPMetadata : public D3PP::files::IMetadataStructure {
    public:
        short BuildRank;
        short JoinRank;
        short ShowRank;
        std::vector<D3PP::world::Teleporter> portals;
        std::vector<D3PP::world::CustomParticle> particles;
        std::vector<unsigned char> history;

        Nbt::TagCompound Read(Nbt::TagCompound metadata) override;
        Nbt::TagCompound Write() override;

    private:
    };
}
#endif //D3PP_D3PPMETADATA_H
