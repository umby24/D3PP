//
// Created by Wande on 1/12/2022.
//

#ifndef D3PP_IMAPPROVIDER_H
#define D3PP_IMAPPROVIDER_H
#include <string>
#include <vector>
#include "common/Vectors.h"
#include "common/MinecraftLocation.h"
#include "world/MapPermissions.h"
#include "world/MapEnvironment.h"

namespace D3PP::world {
    class Teleporter;
    class CustomParticle;

        class IMapProvider {
        public:
            IMapProvider() = default;
            virtual ~IMapProvider()= default;
            
            std::string MapName;
            std::string CreatingUser;
            std::string CreatingService;

            virtual void CreateNew(const Common::Vector3S& size, const std::string& path, const std::string& name)=0;

            virtual bool Save(const std::string& filePath) = 0;
            virtual bool Load(const std::string& filePath) = 0;

            [[nodiscard]] virtual Common::Vector3S GetSize() const = 0;
            virtual void SetSize(const Common::Vector3S& newSize) = 0;

            virtual bool Unload() = 0;
            virtual bool Reload() = 0;

            virtual void SetBlock(const Common::Vector3S& location, const unsigned char& type) = 0;
            virtual unsigned char GetBlock(const Common::Vector3S& location) = 0;

            virtual short GetLastPlayer(const Common::Vector3S& location) = 0;
            virtual void SetLastPlayer(const Common::Vector3S& location, const short& player) = 0;

            virtual void SetBlocks(const std::vector<unsigned char>& blocks) = 0;
            virtual std::vector<unsigned char> GetBlocks() = 0;

            virtual MinecraftLocation GetSpawn() = 0;
            virtual void SetSpawn(const MinecraftLocation& location) = 0;

            virtual MapPermissions GetPermissions() = 0;
            virtual void SetPermissions(const MapPermissions& perms) = 0;

            virtual MapEnvironment GetEnvironment() = 0;
            virtual void SetEnvironment(const MapEnvironment& env) = 0;

            virtual std::vector<D3PP::world::Teleporter> getPortals() = 0;
            virtual void SetPortals(std::vector<D3PP::world::Teleporter> portals) = 0;

            virtual std::vector<CustomParticle> getParticles() = 0;
            virtual void SetParticles(std::vector<CustomParticle> particles) = 0;
        };
    }



#endif //D3PP_IMAPPROVIDER_H
