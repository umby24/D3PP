//
// Created by Wande on 8/10/2022.
//

#include <world/ClassicWorldMapProvider.h>
#include "world/Teleporter.h"
#include "world/CustomParticle.h"
#include "common/D3PPMetadata.h"
#include "common/Logger.h"

D3PP::world::ClassicWorldMapProvider::ClassicWorldMapProvider() {
    CreatingService = "D3PP Server";
    CreatingUser = "D3";
    m_d3meta = std::make_shared<Common::D3PPMetadata>();
    m_cwMap = nullptr;
}

void D3PP::world::ClassicWorldMapProvider::CreateNew(const Common::Vector3S &size, const std::string &path,
                                                     const std::string &name) {
    m_cwMap = std::make_unique<files::ClassicWorld>(size);
    m_cwMap->MapName = name;
    m_cwMap->CreatingService = CreatingService;
    m_cwMap->CreatingUsername = CreatingUser;
    m_currentPath = path;
    MapName = name;
    m_cwMap->metaParsers.insert(std::make_pair("D3PP", m_d3meta));
    m_cwMap->Save(m_currentPath);
}

bool D3PP::world::ClassicWorldMapProvider::Save(const std::string &filePath) {
    if (filePath.empty())
        m_cwMap->Save(m_currentPath);
    else
        m_cwMap->Save(filePath);

    return true;
}

bool D3PP::world::ClassicWorldMapProvider::Load(const std::string &filePath) {
    if (m_cwMap == nullptr) {
        m_cwMap = std::make_unique<files::ClassicWorld>(filePath);
        m_currentPath = filePath;
        m_cwMap->metaParsers.insert(std::make_pair("D3PP", m_d3meta));
    }

    m_currentPath = filePath;

    try {
        m_cwMap->Load();
    } catch (std::exception& e) {
        Logger::LogAdd("ClassicWorldProvider", "Error loading the map at [" + filePath + "]. [Message: " + e.what() + "]", L_ERROR, GLF);
        return false;
    }

    MapName = m_cwMap->MapName;

    return true;
}

D3PP::Common::Vector3S D3PP::world::ClassicWorldMapProvider::GetSize() const {
    if (m_cwMap == nullptr)
        return Common::Vector3S();

    return Common::Vector3S{ m_cwMap->Size.X, m_cwMap->Size.Z, m_cwMap->Size.Y};
}

void D3PP::world::ClassicWorldMapProvider::SetSize(const Common::Vector3S &newSize) {
    if (m_cwMap->Spawn.X > newSize.X)
        m_cwMap->Spawn.X = newSize.X-1;

    if (m_cwMap->Spawn.Y > newSize.Y)
        m_cwMap->Spawn.Y = newSize.Y - 1;

    if (m_cwMap->Spawn.Z > newSize.Z)
        m_cwMap->Spawn.Z = newSize.Z - 1;

    int totalSize = newSize.X * newSize.Y * newSize.Z;
    m_cwMap->BlockData.resize(totalSize);

    m_cwMap->Size = Common::Vector3S{newSize.X, newSize.Z, newSize.Y};

}

bool D3PP::world::ClassicWorldMapProvider::Unload() {
    m_cwMap->BlockData.resize(1);
    m_cwMap->BlockData.shrink_to_fit();
    return true;
}

bool D3PP::world::ClassicWorldMapProvider::Reload() {
    m_cwMap->Load();
    return true;
}

void D3PP::world::ClassicWorldMapProvider::SetBlock(const Common::Vector3S &location, const unsigned char &type) {
    int blockIndex = GetBlockIndex(location.X, location.Y, location.Z);
    if (blockIndex >= m_cwMap->BlockData.size()) return;
    m_cwMap->BlockData[blockIndex] = type;
}

unsigned char D3PP::world::ClassicWorldMapProvider::GetBlock(const Common::Vector3S &location) {
    int blockIndex = GetBlockIndex(location.X, location.Y, location.Z);
    if (blockIndex >= m_cwMap->BlockData.size()) return 255;

    return m_cwMap->BlockData[blockIndex];
}

short D3PP::world::ClassicWorldMapProvider::GetLastPlayer(const Common::Vector3S &location) {
    return 0;
}

void D3PP::world::ClassicWorldMapProvider::SetLastPlayer(const Common::Vector3S &location, const short &player) {
// -- TODO:
}

void D3PP::world::ClassicWorldMapProvider::SetBlocks(const std::vector<unsigned char> &blocks) {
    m_cwMap->BlockData.clear();
    std::copy(blocks.begin(), blocks.end(), std::back_inserter(m_cwMap->BlockData));
}

std::vector<unsigned char> D3PP::world::ClassicWorldMapProvider::GetBlocks() {
    return std::vector<unsigned char>(m_cwMap->BlockData);
}

MinecraftLocation D3PP::world::ClassicWorldMapProvider::GetSpawn() {
    MinecraftLocation result;
    result.SetAsBlockCoords(Common::Vector3S{m_cwMap->Spawn.X, m_cwMap->Spawn.Z, m_cwMap->Spawn.Y});
    result.Rotation = m_cwMap->SpawnRotation;
    result.Look = m_cwMap->SpawnLook;
    return result;
}

void D3PP::world::ClassicWorldMapProvider::SetSpawn(const MinecraftLocation &location) {
    auto blockCoords = location.GetAsBlockCoords();
    m_cwMap->Spawn = Common::Vector3S{blockCoords.X, blockCoords.Z, blockCoords.Y};
    m_cwMap->SpawnRotation = location.Rotation;
    m_cwMap->SpawnLook = location.Look;
}

D3PP::world::MapPermissions D3PP::world::ClassicWorldMapProvider::GetPermissions() {
    MapPermissions currentPerms{0, 0, 0};
    currentPerms.RankBuild = m_d3meta->BuildRank;
    currentPerms.RankJoin = m_d3meta->JoinRank;
    currentPerms.RankShow = m_d3meta->ShowRank;

    return currentPerms;
}

void D3PP::world::ClassicWorldMapProvider::SetPermissions(const MapPermissions &perms) {
    m_d3meta->BuildRank = perms.RankBuild;
    m_d3meta->JoinRank = perms.RankJoin;
    m_d3meta->ShowRank = perms.RankShow;
}

D3PP::world::MapEnvironment D3PP::world::ClassicWorldMapProvider::GetEnvironment() {
    MapEnvironment currentEnv{};
    if (m_cwMap->metaParsers.contains("CPE")) {
        auto cpeSettings = static_pointer_cast<files::CPEMetadata>(m_cwMap->metaParsers.at("CPE"));

        if (cpeSettings->EnvColorsVersion > 0) {
            currentEnv.SkyColor = Utils::Rgb(cpeSettings->SkyColor.X,cpeSettings->SkyColor.Y, cpeSettings->SkyColor.Z);
            currentEnv.CloudColor = Utils::Rgb(cpeSettings->CloudColor.X, cpeSettings->CloudColor.Y, cpeSettings->CloudColor.Z);
            currentEnv.FogColor = Utils::Rgb(cpeSettings->FogColor.X, cpeSettings->FogColor.Y, cpeSettings->FogColor.Z);
            currentEnv.Alight = Utils::Rgb(cpeSettings->AmbientColor.X, cpeSettings->AmbientColor.Y, cpeSettings->AmbientColor.Z);
            currentEnv.DLight = Utils::Rgb(cpeSettings->SunlightColor.X, cpeSettings->SunlightColor.Y, cpeSettings->SunlightColor.Z);
        }

        if (cpeSettings->EnvMapAppearanceVersion > 0) {
            currentEnv.SideLevel = cpeSettings->SideLevel;
            currentEnv.SideBlock = cpeSettings->SideBlock;
            currentEnv.EdgeBlock = cpeSettings->EdgeBlock;
            currentEnv.TextureUrl = cpeSettings->TextureUrl;
        }
        if (cpeSettings->HackControlVersion > 0) {
            currentEnv.CanFly = cpeSettings->CanFly;
            currentEnv.CanClip = cpeSettings->CanClip;
            currentEnv.CanSpeed = cpeSettings->CanSpeed;
            currentEnv.CanRespawn = cpeSettings->CanRespawn;
            currentEnv.CanThirdPerson = cpeSettings->CanThirdPerson;
            currentEnv.CanSetWeather = cpeSettings->CanSetWeather;
            currentEnv.JumpHeight = cpeSettings->JumpHeight;
        }
        if (cpeSettings->EnvMapAspectVersion > 0) {
            if (!cpeSettings->MapEnvProperties.empty())
                currentEnv.SideBlock = static_cast<unsigned char>(cpeSettings->MapEnvProperties.at(0));
            if (cpeSettings->MapEnvProperties.size() > 1)
                currentEnv.EdgeBlock = static_cast<unsigned char>(cpeSettings->MapEnvProperties.at(1));
            if (cpeSettings->MapEnvProperties.size() > 2)
                currentEnv.SideLevel = static_cast<short>(cpeSettings->MapEnvProperties.at(2));
            if (cpeSettings->MapEnvProperties.size() > 3)
                currentEnv.cloudHeight = cpeSettings->MapEnvProperties.at(3);
            if (cpeSettings->MapEnvProperties.size() > 4)
                currentEnv.maxFogDistance = cpeSettings->MapEnvProperties.at(4);
            if (cpeSettings->MapEnvProperties.size() > 5)
                currentEnv.cloudSpeed = cpeSettings->MapEnvProperties.at(5);
            if (cpeSettings->MapEnvProperties.size() > 6)
                currentEnv.weatherSpeed = cpeSettings->MapEnvProperties.at(6);
            if (cpeSettings->MapEnvProperties.size() > 7)
                currentEnv.weatherFade = cpeSettings->MapEnvProperties.at(7);
            if (cpeSettings->MapEnvProperties.size() > 8)
                currentEnv.expoFog = cpeSettings->MapEnvProperties.at(8);
            if (cpeSettings->MapEnvProperties.size() > 9)
                currentEnv.mapSideOffset = cpeSettings->MapEnvProperties.at(9);

            currentEnv.TextureUrl = cpeSettings->TextureUrl;
        }
    }

    return currentEnv;
}

void D3PP::world::ClassicWorldMapProvider::SetEnvironment(const MapEnvironment &env) {
    if (m_cwMap->metaParsers.contains("CPE")) {
        auto cpeSettings = static_pointer_cast<files::CPEMetadata>(m_cwMap->metaParsers.at("CPE"));
        cpeSettings->EnvColorsVersion = 1;
        cpeSettings->SkyColor = Common::Vector3S(Utils::RedVal(env.SkyColor), Utils::GreenVal(env.SkyColor), Utils::BlueVal(env.SkyColor));
        cpeSettings->CloudColor = Common::Vector3S(Utils::RedVal(env.CloudColor), Utils::GreenVal(env.CloudColor), Utils::BlueVal(env.CloudColor));
        cpeSettings->FogColor = Common::Vector3S(Utils::RedVal(env.FogColor), Utils::GreenVal(env.FogColor), Utils::BlueVal(env.FogColor));
        cpeSettings->AmbientColor = Common::Vector3S(Utils::RedVal(env.Alight), Utils::GreenVal(env.Alight), Utils::BlueVal(env.Alight));
        cpeSettings->SunlightColor = Common::Vector3S(Utils::RedVal(env.DLight), Utils::GreenVal(env.DLight), Utils::BlueVal(env.DLight));

        cpeSettings->EnvMapAppearanceVersion = 1;
        cpeSettings->SideLevel = env.SideLevel;
        cpeSettings->SideBlock = env.SideBlock;
        cpeSettings->EdgeBlock = env.EdgeBlock;
        cpeSettings->TextureUrl = env.TextureUrl;
        
        cpeSettings->HackControlVersion = 1;
        cpeSettings->CanFly = env.CanFly;
        cpeSettings->CanClip = env.CanClip;
        cpeSettings->CanSpeed = env.CanSpeed;
        cpeSettings->CanRespawn = env.CanRespawn;
        cpeSettings->CanThirdPerson = env.CanThirdPerson;
        cpeSettings->CanSetWeather = env.CanSetWeather;
        cpeSettings->JumpHeight = env.JumpHeight;

        cpeSettings->EnvMapAspectVersion = 1;
        cpeSettings->MapEnvProperties = std::map<unsigned char, int>();
        cpeSettings->MapEnvProperties.insert(std::make_pair(0, env.SideBlock));
        cpeSettings->MapEnvProperties.insert(std::make_pair(1, env.EdgeBlock));
        cpeSettings->MapEnvProperties.insert(std::make_pair(2, env.SideLevel));
        cpeSettings->MapEnvProperties.insert(std::make_pair(3, env.cloudHeight));
        cpeSettings->MapEnvProperties.insert(std::make_pair(4, env.maxFogDistance));
        cpeSettings->MapEnvProperties.insert(std::make_pair(5, env.cloudSpeed));
        cpeSettings->MapEnvProperties.insert(std::make_pair(6, env.weatherSpeed));
        cpeSettings->MapEnvProperties.insert(std::make_pair(7, env.weatherFade));
        cpeSettings->MapEnvProperties.insert(std::make_pair(8, env.expoFog));
        cpeSettings->MapEnvProperties.insert(std::make_pair(9, env.mapSideOffset));
    }
}

std::vector<D3PP::world::Teleporter> D3PP::world::ClassicWorldMapProvider::getPortals() {
    return m_d3meta->portals;
}

void D3PP::world::ClassicWorldMapProvider::SetPortals(const std::vector<Teleporter> portals) {
    m_d3meta->portals.clear();
    std::copy(portals.begin(), portals.end(), std::back_inserter(m_d3meta->portals));
}

std::vector<D3PP::world::CustomParticle> D3PP::world::ClassicWorldMapProvider::getParticles() {
    return m_d3meta->particles;
}

void D3PP::world::ClassicWorldMapProvider::SetParticles(std::vector<CustomParticle> particles) {
    m_d3meta->particles.clear();
    std::copy(particles.begin(), particles.end(), std::back_inserter(m_d3meta->particles));
}

int D3PP::world::ClassicWorldMapProvider::GetBlockIndex(int x, int z, int y) {
    return (y * m_cwMap->Size.Z + z) * m_cwMap->Size.X + x;
}



