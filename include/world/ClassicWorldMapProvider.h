//
// Created by Wande on 8/10/2022.
//

#ifndef D3PP_CLASSICWORLDMAPPROVIDER_H
#define D3PP_CLASSICWORLDMAPPROVIDER_H
#include "IMapProvider.h"
#include "files/ClassicWorld.h"

namespace D3PP::world {
    class Teleporter;
    class CustomParticle;

    class ClassicWorldMapProvider : public IMapProvider {
    public:
        ClassicWorldMapProvider();
        void CreateNew(const Common::Vector3S& size, const std::string& path, const std::string& name) override;
        bool Save(const std::string& filePath) override;
        bool Load(const std::string& filePath) override;
        Common::Vector3S GetSize() const override;
        void SetSize(const Common::Vector3S& newSize) override;
        bool Unload() override;
        bool Reload() override;
        void SetBlock(const Common::Vector3S& location, const unsigned char& type) override;
        unsigned char GetBlock(const Common::Vector3S& location) override;
        short GetLastPlayer(const Common::Vector3S& location) override;
        void SetLastPlayer(const Common::Vector3S& location, const short& player) override;
        void SetBlocks(const std::vector<unsigned char>& blocks) override;
        std::vector<unsigned char> GetBlocks() override;
        MinecraftLocation GetSpawn() override;
        void SetSpawn(const MinecraftLocation& location) override;
        MapPermissions GetPermissions() override;
        void SetPermissions(const MapPermissions& perms) override;
        MapEnvironment GetEnvironment() override;
        void SetEnvironment(const MapEnvironment& env) override;

        std::vector<D3PP::world::Teleporter> getPortals() override;
        void SetPortals(const std::vector<D3PP::world::Teleporter> portals) override;

        std::vector<CustomParticle> getParticles() override;
        void SetParticles(std::vector<CustomParticle> particles) override;
    private:
        std::string m_currentPath;
        std::unique_ptr<files::ClassicWorld> m_cwMap;
        int GetBlockIndex(int x, int z, int y);
    };
}

#endif //D3PP_CLASSICWORLDMAPPROVIDER_H
