#ifndef TELEPORTER_H
#define TELEPORTER_H
#include <vector>
#include <string>
#include "common/MinecraftLocation.h"


namespace D3PP::world {
    class Teleporter {
    public:
        std::string Name;
        MinecraftLocation OriginStart;
        MinecraftLocation OriginEnd;
        MinecraftLocation Destination;
        std::string DestinationMap;

        Teleporter();

        Teleporter(const MinecraftLocation &start, const MinecraftLocation &end, const MinecraftLocation &dest,
                   const std::string &name, const std::string &destMap);

        bool InRange(const MinecraftLocation &location);

        bool Equals(const Teleporter &other);

        static bool Matches(const MinecraftLocation &location, std::vector<Teleporter> portals);

//    static bool AddTeleporter(std::shared_ptr<D3PP::world::Map> map, std::string id, unsigned short x0, unsigned short x1, unsigned short y0, unsigned short y1, unsigned short z0, unsigned short z1, std::string destUniqueId, int destMapId, float x, float y, float z, float rot, float look);
//    static bool DeleteTeleporter(std::shared_ptr<D3PP::world::Map> map, std::string id);
    };
}

#endif