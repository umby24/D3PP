//
// Created by Wande on 1/12/2022.
//

#include "world/D3MapProvider.h"
#include "world/Teleporter.h"
#include "world/CustomParticle.h"

D3PP::world::D3MapProvider::D3MapProvider() {
    CreatingService = "D3 Server";
    CreatingUser = "D3";
    m_d3map = nullptr;
}

void D3PP::world::D3MapProvider::CreateNew(const D3PP::Common::Vector3S &size, const std::string &path,
                                           const std::string &name) {
    m_d3map = std::make_unique<files::D3Map>(path, name, size);
    MapName = name;
    auto defaultEnv = MapEnvironment();
    defaultEnv.SideLevel = size.Z/2;
    m_currentPath = path;
    SetEnvironment(defaultEnv);
    SetPermissions(MapPermissions {0, 0, 0});

    m_d3map->Save();
}

bool D3PP::world::D3MapProvider::Save(const std::string &filePath) {
    if (filePath == "")
        return m_d3map->Save();

    return m_d3map->Save(filePath);
}

bool D3PP::world::D3MapProvider::Load(const std::string &filePath) {
    if (m_d3map == nullptr) {
        m_d3map = std::make_unique<files::D3Map>(filePath);
        m_currentPath = filePath;
    }
    bool loadResult = true;

    if (!filePath.empty())
        loadResult = m_d3map->Load(filePath);
    else
        loadResult = m_d3map->Load(m_currentPath);

    if (filePath == m_currentPath)
        MapName = m_d3map->Name;

    return loadResult;
}

D3PP::Common::Vector3S D3PP::world::D3MapProvider::GetSize() const {
    return Common::Vector3S(m_d3map->MapSize);
}

void D3PP::world::D3MapProvider::SetSize(const D3PP::Common::Vector3S &newSize) {
    m_d3map->Resize(newSize);
}

bool D3PP::world::D3MapProvider::Unload() {
    m_d3map->MapData.resize(1);
    return true;
}

bool D3PP::world::D3MapProvider::Reload() {
    return Load("");
}

void D3PP::world::D3MapProvider::SetBlock(const D3PP::Common::Vector3S &location, const unsigned char &type) {
    m_d3map->SetBlock(location, type);
}

unsigned char D3PP::world::D3MapProvider::GetBlock(const D3PP::Common::Vector3S& location) {
    return m_d3map->GetBlock(location);
}

void D3PP::world::D3MapProvider::SetBlocks(const std::vector<unsigned char> &blocks) {
    m_d3map->MapData.clear();
    std::copy(blocks.begin(), blocks.end(), std::back_inserter(m_d3map->MapData));
}

std::vector<unsigned char> D3PP::world::D3MapProvider::GetBlocks() {
    return std::vector<unsigned char>(m_d3map->MapData);
}

MinecraftLocation D3PP::world::D3MapProvider::GetSpawn() {
    return MinecraftLocation(m_d3map->MapSpawn);
}

void D3PP::world::D3MapProvider::SetSpawn(const MinecraftLocation &location) {
    m_d3map->MapSpawn = location;
}

D3PP::world::MapPermissions D3PP::world::D3MapProvider::GetPermissions() {
    return D3PP::world::MapPermissions { m_d3map->BuildRank, m_d3map->JoinRank, m_d3map->ShowRank };
}

void D3PP::world::D3MapProvider::SetPermissions(const D3PP::world::MapPermissions &perms) {
    m_d3map->BuildRank = perms.RankBuild;
    m_d3map->JoinRank = perms.RankJoin;
    m_d3map->ShowRank = perms.RankShow;
}

short D3PP::world::D3MapProvider::GetLastPlayer(const D3PP::Common::Vector3S &location) {
    return m_d3map->GetBlockLastPlayer(location);
}

void D3PP::world::D3MapProvider::SetLastPlayer(const D3PP::Common::Vector3S &location, const short &player) {
    m_d3map->SetBlockLastPlayer(location, player);
}

D3PP::world::MapEnvironment D3PP::world::D3MapProvider::GetEnvironment() {
    MapEnvironment currentEnv{};
    currentEnv.SkyColor = m_d3map->SkyColor;
    currentEnv.CloudColor = m_d3map->CloudColor;
    currentEnv.FogColor = m_d3map->FogColor;
    currentEnv.Alight = m_d3map->alight;
    currentEnv.DLight = m_d3map->dlight;
    currentEnv.CanFly = m_d3map->Flying;
    currentEnv.CanClip = m_d3map->NoClip;
    currentEnv.CanSpeed = m_d3map->Speeding;
    currentEnv.CanRespawn = m_d3map->SpawnControl;
    currentEnv.CanThirdPerson = m_d3map->ThirdPerson;
    currentEnv.CanSetWeather = m_d3map->Weather;
    currentEnv.JumpHeight = m_d3map->JumpHeight;
    currentEnv.SideLevel = m_d3map->SideLevel;
    currentEnv.SideBlock = m_d3map->SideBlock;
    currentEnv.EdgeBlock = m_d3map->EdgeBlock;
    currentEnv.TextureUrl = m_d3map->TextureUrl;

    return currentEnv;
}

void D3PP::world::D3MapProvider::SetEnvironment(const D3PP::world::MapEnvironment &env) {
    m_d3map->SkyColor =  env.SkyColor;
    m_d3map->CloudColor =  env.CloudColor;
    m_d3map->FogColor =  env.FogColor;
    m_d3map->alight =  env.Alight;
    m_d3map->dlight =  env.DLight;
    m_d3map->Flying =  env.CanFly;
    m_d3map->NoClip =  env.CanClip;
    m_d3map->Speeding =  env.CanSpeed;
    m_d3map->SpawnControl =  env.CanRespawn;
    m_d3map->ThirdPerson =  env.CanThirdPerson;
    m_d3map->Weather =  env.CanSetWeather;
    m_d3map->JumpHeight =  env.JumpHeight;
    m_d3map->SideLevel =  env.SideLevel;
    m_d3map->SideBlock =  env.SideBlock;
    m_d3map->EdgeBlock =  env.EdgeBlock;
    m_d3map->TextureUrl =  env.TextureUrl;
}

void D3PP::world::D3MapProvider::SetPortals(const std::vector<D3PP::world::Teleporter> portals) {
    std::vector<files::MapTeleporterElement> mtes;

    for(const auto &item : portals) {
        auto startBlockCoords = item.OriginStart.GetAsBlockCoords();
        auto endBlockCoords = item.OriginEnd.GetAsBlockCoords();
        auto destBlockCoords = item.Destination.GetAsBlockCoords();

        files::MapTeleporterElement newMte;

       newMte.Id = item.Name;
       newMte.X0 = startBlockCoords.X;
       newMte.Y0 = startBlockCoords.Y;
       newMte.Z0 = startBlockCoords.Z;
       newMte.X1 = endBlockCoords.X;
       newMte.Y1 = endBlockCoords.Y;
       newMte.Z1 = endBlockCoords.Z;
       newMte.DestMapUniqueId = item.DestinationMap;
       newMte.DestMapId = -1;
       newMte.DestX = destBlockCoords.X;
       newMte.DestY = destBlockCoords.Y;
       newMte.DestZ = destBlockCoords.Z;
       newMte.DestRot = item.Destination.Rotation;
       newMte.DestLook = item.Destination.Look;

        mtes.push_back(newMte);
    }
}

std::vector<D3PP::world::Teleporter> D3PP::world::D3MapProvider::getPortals() {
    std::vector<D3PP::world::Teleporter> result;
    auto mapPortals = m_d3map->getPortals();
    for(const auto &i : mapPortals) {
        MinecraftLocation origin{};
        origin.SetAsBlockCoords(Common::Vector3S(i.X0, i.Y0, i.Z0));
        MinecraftLocation end{};
        end.SetAsBlockCoords(Common::Vector3S(i.X1, i.Y1, i.Z1));
        MinecraftLocation dest{};
        dest.SetAsBlockCoords(Common::Vector3S(static_cast<short>(i.DestX), i.DestY, i.DestZ));
        dest.Rotation = i.DestRot;
        dest.Look = i.DestLook;

        D3PP::world::Teleporter newTp;
        newTp.Destination = dest;
        newTp.OriginStart = origin;
        newTp.OriginEnd = end;
        newTp.Name = i.Id;
        newTp.DestinationMap = i.DestMapUniqueId;
        result.push_back(newTp);
    }

    return result;
}

std::vector<D3PP::world::CustomParticle> D3PP::world::D3MapProvider::getParticles() {
    return m_d3map->GetParticles();
}

void D3PP::world::D3MapProvider::SetParticles(std::vector<D3PP::world::CustomParticle> particles) {
    if (m_d3map != nullptr)
        m_d3map->SetParticles(particles);
}


