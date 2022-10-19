//
// Created by Wande on 8/10/2022.
//

#include <world/ClassicWorldMapProvider.h>
#include "world/Teleporter.h"
#include "world/CustomParticle.h"
#include "common/D3PPMetadata.h"

D3PP::world::ClassicWorldMapProvider::ClassicWorldMapProvider() {
    CreatingService = "D3PP Server";
    CreatingUser = "D3";
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
    }

    m_currentPath = filePath;

    try {
        m_cwMap->Load();
    } catch (std::exception& e) {
        return false;
    }

    MapName = m_cwMap->MapName;

    return true;
}

D3PP::Common::Vector3S D3PP::world::ClassicWorldMapProvider::GetSize() const {
    if (m_cwMap == nullptr)
        return Common::Vector3S();

    return m_cwMap->Size;
}

void D3PP::world::ClassicWorldMapProvider::SetSize(const D3PP::Common::Vector3S &newSize) {
    if (m_cwMap->Spawn.X > newSize.X)
        m_cwMap->Spawn.X = newSize.X-1;

    if (m_cwMap->Spawn.Y > newSize.Y)
        m_cwMap->Spawn.Y = newSize.Y - 1;

    if (m_cwMap->Spawn.Z > newSize.Z)
        m_cwMap->Spawn.Z = newSize.Z - 1;

    int totalSize = newSize.X * newSize.Y * newSize.Z;
    m_cwMap->BlockData.resize(totalSize);
    m_cwMap->Size = newSize;

}

bool D3PP::world::ClassicWorldMapProvider::Unload() {
    m_cwMap->BlockData.resize(1);
    return true;
}

bool D3PP::world::ClassicWorldMapProvider::Reload() {
    m_cwMap->Load();
    return true;
}

void D3PP::world::ClassicWorldMapProvider::SetBlock(const D3PP::Common::Vector3S &location, const unsigned char &type) {
    int blockIndex = GetBlockIndex(location.X, location.Y, location.Z);
    if (blockIndex > m_cwMap->BlockData.size()) return;
    m_cwMap->BlockData[blockIndex] = type;
}

unsigned char D3PP::world::ClassicWorldMapProvider::GetBlock(const D3PP::Common::Vector3S &location) {
    int blockIndex = GetBlockIndex(location.X, location.Y, location.Z);
    if (blockIndex > m_cwMap->BlockData.size()) return 255;

    return m_cwMap->BlockData[blockIndex];
}

short D3PP::world::ClassicWorldMapProvider::GetLastPlayer(const D3PP::Common::Vector3S &location) {
    return 0;
}

void D3PP::world::ClassicWorldMapProvider::SetLastPlayer(const D3PP::Common::Vector3S &location, const short &player) {
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
    result.SetAsBlockCoords(m_cwMap->Spawn);
    result.Rotation = m_cwMap->SpawnRotation;
    result.Look = m_cwMap->SpawnLook;
    return result;
}

void D3PP::world::ClassicWorldMapProvider::SetSpawn(const MinecraftLocation &location) {
    m_cwMap->Spawn = location.GetAsBlockCoords();
    m_cwMap->SpawnRotation = location.Rotation;
    m_cwMap->SpawnLook = location.Look;
}

D3PP::world::MapPermissions D3PP::world::ClassicWorldMapProvider::GetPermissions() {
    // -- TODO:
    D3PP::world::MapPermissions currentPerms{0, 0, 0};
    if (m_cwMap->metaParsers.contains("D3PP")) {
        auto d3Settings = static_pointer_cast<Common::D3PPMetadata>(m_cwMap->metaParsers.at("D3PP"));
        currentPerms.RankBuild = d3Settings->BuildRank;
        currentPerms.RankJoin = d3Settings->JoinRank;
        currentPerms.RankShow = d3Settings->ShowRank;
    }

    return currentPerms;
}

void D3PP::world::ClassicWorldMapProvider::SetPermissions(const D3PP::world::MapPermissions &perms) {
    if (!m_cwMap->metaParsers.contains("D3PP")) {
        m_cwMap->metaParsers.insert(std::make_pair("D3PP", std::make_shared<Common::D3PPMetadata>()));
    }
    auto d3Settings = static_pointer_cast<Common::D3PPMetadata>(m_cwMap->metaParsers.at("D3PP"));
    d3Settings->BuildRank = perms.RankBuild;
    d3Settings->JoinRank = perms.RankJoin;
    d3Settings->ShowRank = perms.RankShow;

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
    }

//    currentEnv.CanFly = m_d3map->Flying;
//    currentEnv.CanClip = m_d3map->NoClip;
//    currentEnv.CanSpeed = m_d3map->Speeding;
//    currentEnv.CanRespawn = m_d3map->SpawnControl;
//    currentEnv.CanThirdPerson = m_d3map->ThirdPerson;
//    currentEnv.CanSetWeather = m_d3map->Weather;
//    currentEnv.JumpHeight = m_d3map->JumpHeight;
//
//    currentEnv.cloudHeight = m_d3map->cloudHeight;
//    currentEnv.maxFogDistance = m_d3map->maxFogDistance;
//    currentEnv.cloudSpeed = m_d3map->cloudSpeed;
//    currentEnv.weatherSpeed = m_d3map->weatherSpeed;
//    currentEnv.weatherFade = m_d3map->weatherFade;
//    currentEnv.expoFog = m_d3map->expoFog;
//    currentEnv.mapSideOffset = -m_d3map->mapSideOffset;
    return currentEnv;
}

void D3PP::world::ClassicWorldMapProvider::SetEnvironment(const D3PP::world::MapEnvironment &env) {
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
    }
}

std::vector<D3PP::world::Teleporter> D3PP::world::ClassicWorldMapProvider::getPortals() {
    return std::vector<D3PP::world::Teleporter>();
}

void D3PP::world::ClassicWorldMapProvider::SetPortals(const std::vector<D3PP::world::Teleporter> portals) {

}

std::vector<D3PP::world::CustomParticle> D3PP::world::ClassicWorldMapProvider::getParticles() {
    return std::vector<D3PP::world::CustomParticle>();
}

void D3PP::world::ClassicWorldMapProvider::SetParticles(std::vector<D3PP::world::CustomParticle> particles) {

}

int D3PP::world::ClassicWorldMapProvider::GetBlockIndex(int x, int z, int y) {
    return (y * m_cwMap->Size.Z + z) * m_cwMap->Size.X + x;
}



