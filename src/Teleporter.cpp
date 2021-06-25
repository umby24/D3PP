#include "Teleporter.h"

bool Teleporter::AddTeleporter(std::shared_ptr<Map> map, std::string id, unsigned short x0, unsigned short x1, unsigned short y0, unsigned short y1, unsigned short z0, unsigned short z1, std::string destUniqueId, int destMapId, float x, float y, float z, float rot, float look) {
    if (x0 > x1) {
        unsigned short x2 = x0;
        x0 = x1;
        x1 = x2;
    }
    if (y0 > y1) {
        unsigned short y2 = y0;
        y0 = y1;
        y1 = y2;
    }
    if (z0 > z1) {
        unsigned short z2 = z0;
        z0 = z1;
        z1 = z2;
    }
    MapTeleporterElement mte;
    mte.Id = id;
    mte.X0 = x0;
    mte.X1 = x1;
    mte.Y0 = y0;
    mte.Y1 = y1;
    mte.Z0 = z0;
    mte.Z1 = z1;
    mte.DestLook = look;
    mte.DestRot = rot;
    mte.DestX = x;
    mte.DestY = y;
    mte.DestZ = z;
    mte.DestMapUniqueId = destMapId;
    mte.DestMapId = destMapId;
    map->data.Teleporter.insert(std::make_pair(id, mte));
    return true;
}

bool Teleporter::DeleteTeleporter(std::shared_ptr<Map> map, std::string id) {
    map->data.Teleporter.erase(id);
    return true;
}