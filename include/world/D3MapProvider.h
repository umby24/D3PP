//
// Created by Wande on 1/12/2022.
//

#ifndef D3PP_D3MAPPROVIDER_H
#define D3PP_D3MAPPROVIDER_H
#include "IMapProvider.h"
#include "files/D3Map.h"

namespace D3PP::world {
    class D3MapProvider : public IMapProvider {
    public:
        D3MapProvider();
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
    private:
        std::string m_currentPath;
        std::unique_ptr<files::D3Map> m_d3map;
    };
}
#endif //D3PP_D3MAPPROVIDER_H
