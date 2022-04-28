#include "world/Teleporter.h"
#include "common/MinecraftLocation.h"

namespace D3PP::world {
    Teleporter::Teleporter() : OriginStart{}, OriginEnd{}, Destination{} {
        Name = "";
        DestinationMap = "";
    }

    Teleporter::Teleporter(const MinecraftLocation &start, const MinecraftLocation &end, const MinecraftLocation &dest,
                           const std::string &name, const std::string &destMap) {
        OriginStart = start;
        OriginEnd = end;
        Destination = dest;
        Name = name;
        DestinationMap = destMap;
    }

    bool Teleporter::InRange(const MinecraftLocation &location) {
        auto currentBlock = location.GetAsBlockCoords();
        auto startBlock = OriginStart.GetAsBlockCoords();
        auto endBlock = OriginEnd.GetAsBlockCoords();

        bool xInRange = currentBlock.X >= startBlock.X && currentBlock.X <= endBlock.X;
        bool yInRange = currentBlock.Y >= startBlock.Y && currentBlock.Y <= endBlock.Y;
        bool zInRange = currentBlock.Z >= startBlock.Z && currentBlock.Z <= endBlock.Z;

        return xInRange && yInRange && zInRange;
    }

    bool Teleporter::Equals(const Teleporter &other) {
        return (OriginStart == other.OriginStart) && (Destination == other.Destination) &&
               (OriginEnd == other.OriginEnd);
    }

    bool Teleporter::Matches(const MinecraftLocation &location, std::vector<Teleporter> portals) {
        return false;
    }
}

