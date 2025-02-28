#include <Utils.h>
#include "CPE.h"
#include "common/Configuration.h"
#include "Client.h"
#include "world/Player.h"
#include "world/Entity.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "network/Server.h"
#include "network/packets/DefineEffectPacket.h"
#include "network/packets/SetMapEnvPropertyPacket.h"
#include "network/packets/SetMapEnvUrlPacket.h"
#include "network/packets/SetTextColor.h"
#include "network/Packets.h"
#include "world/Map.h"
#include "world/MapMain.h"
#include "CustomBlocks.h"
#include "world/CustomParticle.h"

using namespace D3PP::world;
using namespace D3PP::Common;

void CPE::PreLoginExtensions(const std::shared_ptr<IMinecraftClient>& client) {
    auto concrete = std::static_pointer_cast<NetworkClient>(client);
    auto playerConcrete = std::static_pointer_cast<D3PP::world::Player>(client->GetPlayerInstance());

    if (GetClientExtVersion(client, CLICK_DISTANCE_EXT_NAME) == 1) {
        // -- do click distance things
        Packets::SendClickDistance(concrete, Configuration::GenSettings.ClickDistance);
    }
    if (GetClientExtVersion(client, CUSTOM_BLOCKS_EXT_NAME) == 1) {
        Packets::SendCustomBlockSupportLevel(concrete, 1);
    } else {
        Client::Login(client->GetId(), concrete->GetLoginName(), playerConcrete->MPPass, playerConcrete->ClientVersion);
    }
}

void CPE::DuringMapActions(const std::shared_ptr<IMinecraftClient>& client) {
    auto concrete = std::static_pointer_cast<NetworkClient>(client);

    CustomBlocks* cb = CustomBlocks::GetInstance();
    std::vector<BlockDefinition> blocks = cb->GetBlocks();

    if (GetClientExtVersion(client, BLOCK_DEFS_EXT_NAME) == 1 && GetClientExtVersion(client, BLOCK_DEFS_EXTENDED_EXT_NAME) != 2) {
        for(auto const &b : blocks) {
            Packets::SendDefineBlock(concrete, b);
        }
    }
    else if (GetClientExtVersion(client, BLOCK_DEFS_EXTENDED_EXT_NAME) == 2) {
        for (auto const& b : blocks) {
            if (b.shape == 0)
                Packets::SendDefineBlock(concrete, b); // -- Sprites must be defined using normal define block packet first :) 
            else
                Packets::SendDefineBlockExt(concrete, b);
        }
    }
}

int CPE::GetClientExtVersion(const std::shared_ptr<IMinecraftClient>& client, const std::string& extension) {
    if (client == nullptr)
        return 0;

    std::shared_ptr<NetworkClient> c = std::static_pointer_cast<NetworkClient>(client);
    if (c == nullptr)
        return 0;

    if (!c->CPE)
        return 0;

    if (!c->Extensions.contains(extension)) {
        return 0;
    }

    return c->Extensions.at(extension);
}

void CPE::AfterMapActions(const std::shared_ptr<IMinecraftClient>& client) {
    if (!client->GetLoggedIn())
        return;

    MapMain* mm = MapMain::GetInstance();
    auto concrete = std::static_pointer_cast<NetworkClient>(client);
    std::shared_ptr<Map> clientMap = mm->GetPointer(concrete->GetPlayerInstance()->GetEntity()->MapID);
    MapEnvironment perms = clientMap->GetMapEnvironment();

    if (GetClientExtVersion(client, ENV_COLORS_EXT_NAME) == 1) {
        // -- Set map colors
//        if (clientMap->data.ColorsSet) {
            
            int red = Utils::RedVal(perms.SkyColor);
            int green = Utils::GreenVal(perms.SkyColor);
            int blue = Utils::BlueVal(perms.SkyColor);
            Packets::SendSetEnvironmentColors(concrete, 0, red, green, blue);

            red = Utils::RedVal(perms.CloudColor);
            green = Utils::GreenVal(perms.CloudColor);
            blue = Utils::BlueVal(perms.CloudColor);
            Packets::SendSetEnvironmentColors(concrete, 1, red, green, blue);

            red = Utils::RedVal(perms.FogColor);
            green = Utils::GreenVal(perms.FogColor);
            blue = Utils::BlueVal(perms.FogColor);
            Packets::SendSetEnvironmentColors(concrete, 2, red, green, blue);

            red = Utils::RedVal(perms.Alight);
            green = Utils::GreenVal(perms.Alight);
            blue = Utils::BlueVal(perms.Alight);
            Packets::SendSetEnvironmentColors(concrete, 3, red, green, blue);

            red = Utils::RedVal(perms.DLight);
            green = Utils::GreenVal(perms.DLight);
            blue = Utils::BlueVal(perms.DLight);
            Packets::SendSetEnvironmentColors(concrete, 4, red, green, blue);
//        }
    }
    if (GetClientExtVersion(client, ENV_APPEARANCE_EXT_NAME) == 1) {
        // -- set map customization
//        if (clientMap->data.CustomAppearance) {
            Packets::SendEnvMapAppearance(std::static_pointer_cast<NetworkClient>(client), perms.TextureUrl, perms.SideBlock, perms.EdgeBlock, perms.SideLevel);
//        }
    }
    if (GetClientExtVersion(client, MAP_ASPECT_EXT_NAME) == 1) {
        if (perms.TextureUrl.starts_with("http")) {
            D3PP::network::SetMapEnvUrlPacket urlPacket;
            urlPacket.textureUrl = perms.TextureUrl;
            Utils::padTo(urlPacket.textureUrl, 64);
            client->SendPacket(urlPacket);
        }

        D3PP::network::SetMapEnvPropertyPacket prop;
        prop.propertyType = static_cast<unsigned char>(D3PP::network::MapEnvProperty::SideBlockId);
        prop.propertyValue = perms.SideBlock;
        client->SendPacket(prop);

        prop.propertyType = static_cast<unsigned char>(D3PP::network::MapEnvProperty::EdgeBlockId);
        prop.propertyValue = perms.EdgeBlock;
        client->SendPacket(prop);

        prop.propertyType = static_cast<unsigned char>(D3PP::network::MapEnvProperty::EdgeHeight);
        prop.propertyValue = perms.SideLevel;
        client->SendPacket(prop);

        prop.propertyType = static_cast<unsigned char>(D3PP::network::MapEnvProperty::CloudHeight);
        prop.propertyValue = perms.cloudHeight;
        client->SendPacket(prop);

        prop.propertyType = static_cast<unsigned char>(D3PP::network::MapEnvProperty::FogDistance);
        prop.propertyValue = perms.maxFogDistance;
        client->SendPacket(prop);

        prop.propertyType = static_cast<unsigned char>(D3PP::network::MapEnvProperty::CloudSpeed);
        prop.propertyValue = perms.cloudSpeed;
        client->SendPacket(prop);

        prop.propertyType = static_cast<unsigned char>(D3PP::network::MapEnvProperty::WeatherSpeed);
        prop.propertyValue = perms.weatherSpeed;
        client->SendPacket(prop);

        prop.propertyType = static_cast<unsigned char>(D3PP::network::MapEnvProperty::WeatherFade);
        prop.propertyValue = perms.weatherFade;
        client->SendPacket(prop);

        prop.propertyType = static_cast<unsigned char>(D3PP::network::MapEnvProperty::ExpoFog);
        prop.propertyValue = perms.expoFog;
        client->SendPacket(prop);

        prop.propertyType = static_cast<unsigned char>(D3PP::network::MapEnvProperty::SideOffset);
        prop.propertyValue = perms.mapSideOffset;
        client->SendPacket(prop);
    }
    if (GetClientExtVersion(client, HACKCONTROL_EXT_NAME) == 1) {
        // -- Set map permissions

        Packets::SendHackControl(std::static_pointer_cast<NetworkClient>(client), perms.CanFly, perms.CanClip, perms.CanSpeed, perms.CanRespawn, perms.CanThirdPerson, perms.JumpHeight);
    }
    if (GetClientExtVersion(client, SELECTION_CUBOID_EXT_NAME) == 1) {
        // -- Send map defined selections
    }
    if (GetClientExtVersion(client, EXT_WEATHER_CONTROL_EXT_NAME) == 1) {
        // -- Send current map weather
    }
    if (GetClientExtVersion(client, CUSTOM_PARTICLES_EXT_NAME) == 1) {
        for (const auto& p : clientMap->Particles) {
            D3PP::network::DefineEffectPacket effectPacket;
            effectPacket.effectId = p.effectId;
            effectPacket.U1 = p.U1;
            effectPacket.V1 = p.V1;
            effectPacket.U2 = p.U2;
            effectPacket.V2 = p.V2;
            effectPacket.redTint = p.redTint;
            effectPacket.blueTint = p.blueTint;
            effectPacket.greenTint = p.greenTint;
            effectPacket.frameCount = p.frameCount;
            effectPacket.particleCount = p.particleCount;
            effectPacket.size = p.size;
            effectPacket.sizeVariation = p.sizeVariation;
            effectPacket.spread = p.spread;
            effectPacket.speed = p.speed;
            effectPacket.gravity = p.gravity;
            effectPacket.baseLifetime = p.baseLifetime;
            effectPacket.lifetimeVariation = p.lifetimeVariation;
            effectPacket.collideFlags = p.collideFlags;
            effectPacket.fullBright = p.fullBright;
            client->SendPacket(effectPacket);
        }
    }

    if (GetClientExtVersion(client, TEXT_COLORS_EXT_NAME) == 1) {
        for (const auto& cust : Configuration::textSettings.colors) {
            D3PP::network::SetTextColorPacket p;
            p.code = cust.character.at(0);
            p.red = cust.redVal;
            p.green = cust.greenVal;
            p.blue = cust.blueVal;
            client->SendPacket(p);
        }
    }

    AfterLoginActions(client);
}

void CPE::PostEntityActions(const std::shared_ptr<IMinecraftClient>& client, const std::shared_ptr<Entity>& postEntity) {
    auto concrete = std::static_pointer_cast<NetworkClient>(client);
    if (concrete->LoggedIn && GetClientExtVersion(client, CHANGE_MODEL_EXT_NAME) == 1 && postEntity->model != "Default") {
        Packets::SendChangeModel(concrete, postEntity->ClientId, postEntity->model);
    }
}

void CPE::AfterLoginActions(const std::shared_ptr<IMinecraftClient>& client) {
    MapMain* mapMain = MapMain::GetInstance();
    auto concrete = std::static_pointer_cast<NetworkClient>(client);
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(client->GetId(), true);
    std::shared_ptr<Map> clientMap = mapMain->GetPointer(clientEntity->MapID);
    int clientId = client->GetId();
    std::string loginName = client->GetLoginName();
    std::string prettyName = Entity::GetDisplayname(clientEntity->Id);
    int extVersion = GetClientExtVersion(client, EXT_PLAYER_LIST_EXT_NAME);
    int tempNameId = concrete->GetPlayerInstance()->GetNameId();

    std::shared_lock lock(D3PP::network::Server::roMutex, std::defer_lock);
    for(auto const &nc : D3PP::network::Server::roClients) {
        if (!nc->GetLoggedIn())
            continue;

        if (nc->GetId() != clientId) {
            if (GetClientExtVersion(nc, EXT_PLAYER_LIST_EXT_NAME) == 2) {
                auto concrete2 = std::static_pointer_cast<NetworkClient>(nc);
                Packets::SendExtAddPlayerName(concrete2, tempNameId, loginName, prettyName, clientMap->Name(), 0);
            }
            if (extVersion == 2) {
                std::shared_ptr<Map> dudesMap = mapMain->GetPointer(nc->GetPlayerInstance()->GetEntity()->MapID);
                Packets::SendExtAddPlayerName(concrete, nc->GetPlayerInstance()->GetNameId(), nc->GetPlayerInstance()->GetLoginName(), Entity::GetDisplayname(nc->GetPlayerInstance()->GetEntity()->Id), dudesMap->Name(), 0);
            }
        } else {
            if (extVersion == 2) {
                Packets::SendExtAddPlayerName(concrete, 255, loginName, prettyName, clientMap->Name(), 0);
            }
        }
    }
}