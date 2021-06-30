#ifndef TELEPORTER_H
#define TELEPORTER_H
#include <map>
#include <string>
#include <memory>

class Map;

class Teleporter {
    static bool AddTeleporter(std::shared_ptr<Map> map, std::string id, unsigned short x0, unsigned short x1, unsigned short y0, unsigned short y1, unsigned short z0, unsigned short z1, std::string destUniqueId, int destMapId, float x, float y, float z, float rot, float look);
    static bool DeleteTeleporter(std::shared_ptr<Map> map, std::string id);
};
#endif