#include <System.h>
#include <Utils.h>
#include "CPE.h"
#include "common/Configuration.h"
#include "Client.h"
#include "world/Player.h"
#include "world/Entity.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "network/Packets.h"
#include "world/Map.h"
#include "CustomBlocks.h"

void CPE::PreLoginExtensions(std::shared_ptr<IMinecraftClient> client) {
    auto concrete = std::static_pointer_cast<NetworkClient>(client);

    if (GetClientExtVersion(client, CLICK_DISTANCE_EXT_NAME) == 1) {
        // -- do click distance things
        Packets::SendClickDistance(concrete, Configuration::GenSettings.ClickDistance);
    }
    if (GetClientExtVersion(client, CUSTOM_BLOCKS_EXT_NAME) == 1) {
        Packets::SendCustomBlockSupportLevel(concrete, 1);
    } else {
        Client::Login(client->GetId(), concrete->player->LoginName, concrete->player->MPPass, concrete->player->ClientVersion);
    }
}

void CPE::DuringMapActions(std::shared_ptr<IMinecraftClient> client) {
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

int CPE::GetClientExtVersion(std::shared_ptr<IMinecraftClient> client, std::string extension) {
    std::shared_ptr<NetworkClient> c = std::static_pointer_cast<NetworkClient>(client);
    if (!c->CPE)
        return 0;

    if (c->Extensions.find(extension) == c->Extensions.end()) {
        return 0;
    }

    return c->Extensions[extension];
}

void CPE::AfterMapActions(std::shared_ptr<IMinecraftClient> client) {
    MapMain* mm = MapMain::GetInstance();
    auto concrete = std::static_pointer_cast<NetworkClient>(client);
    std::shared_ptr<Map> clientMap = mm->GetPointer(concrete->player->tEntity->MapID);

    if (GetClientExtVersion(client, ENV_COLORS_EXT_NAME) == 1) {
        // -- Set map colors
        if (clientMap->data.ColorsSet) {
            int red = Utils::RedVal(clientMap->data.SkyColor);
            int green = Utils::GreenVal(clientMap->data.SkyColor);
            int blue = Utils::BlueVal(clientMap->data.SkyColor);
            Packets::SendSetEnvironmentColors(concrete, 0, red, green, blue);

            red = Utils::RedVal(clientMap->data.CloudColor);
            green = Utils::GreenVal(clientMap->data.CloudColor);
            blue = Utils::BlueVal(clientMap->data.CloudColor);
            Packets::SendSetEnvironmentColors(concrete, 1, red, green, blue);

            red = Utils::RedVal(clientMap->data.FogColor);
            green = Utils::GreenVal(clientMap->data.FogColor);
            blue = Utils::BlueVal(clientMap->data.FogColor);
            Packets::SendSetEnvironmentColors(concrete, 2, red, green, blue);

            red = Utils::RedVal(clientMap->data.alight);
            green = Utils::GreenVal(clientMap->data.alight);
            blue = Utils::BlueVal(clientMap->data.alight);
            Packets::SendSetEnvironmentColors(concrete, 3, red, green, blue);

            red = Utils::RedVal(clientMap->data.dlight);
            green = Utils::GreenVal(clientMap->data.dlight);
            blue = Utils::BlueVal(clientMap->data.dlight);
            Packets::SendSetEnvironmentColors(concrete, 4, red, green, blue);
        }
    }
    if (GetClientExtVersion(client, ENV_APPEARANCE_EXT_NAME) == 1) {
        // -- set map customization
        if (clientMap->data.CustomAppearance) {
            Packets::SendEnvMapAppearance(std::static_pointer_cast<NetworkClient>(client), clientMap->data.CustomURL, clientMap->data.SideBlock, clientMap->data.EdgeBlock, clientMap->data.SideLevel);
        }
    }
    if (GetClientExtVersion(client, HACKCONTROL_EXT_NAME) == 1) {
        // -- Set map permissions
        Packets::SendHackControl(std::static_pointer_cast<NetworkClient>(client), clientMap->data.Flying, clientMap->data.NoClip, clientMap->data.Speeding, clientMap->data.SpawnControl, clientMap->data.ThirdPerson, clientMap->data.JumpHeight);
    }
    if (GetClientExtVersion(client, SELECTION_CUBOID_EXT_NAME) == 1) {
        // -- Send map defined selections
    }
    if (GetClientExtVersion(client, EXT_WEATHER_CONTROL_EXT_NAME) == 1) {
        // -- Send current map weather
    }

    AfterLoginActions(client);
}

void CPE::PostEntityActions(std::shared_ptr<IMinecraftClient> client, std::shared_ptr<Entity> postEntity) {
    auto concrete = std::static_pointer_cast<NetworkClient>(client);
    if (concrete->LoggedIn && GetClientExtVersion(client, CHANGE_MODEL_EXT_NAME) == 1 && postEntity->model != "Default") {
        Packets::SendChangeModel(concrete, postEntity->ClientId, postEntity->model);
    }
}

void CPE::AfterLoginActions(std::shared_ptr<IMinecraftClient> client) {
    Network* nm = Network::GetInstance();
    MapMain* mapMain = MapMain::GetInstance();
    auto concrete = std::static_pointer_cast<NetworkClient>(client);
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(client->GetId(), true);
    std::shared_ptr<Map> clientMap = mapMain->GetPointer(clientEntity->MapID);
    int clientId = client->GetId();
    std::string loginName = client->GetLoginName();
    std::string prettyName = Entity::GetDisplayname(clientEntity->Id);
    int extVersion = CPE::GetClientExtVersion(client, EXT_PLAYER_LIST_EXT_NAME);
    int tempNameId = concrete->player->NameId;

    for(auto const &nc : nm->roClients) {
        if (!nc->LoggedIn)
            continue;

        if (nc->GetId() != clientId) {
            if (CPE::GetClientExtVersion(nc, EXT_PLAYER_LIST_EXT_NAME) == 2) {
                Packets::SendExtAddPlayerName(nc, tempNameId, loginName, prettyName, clientMap->data.Name, 0);
            }
            if (extVersion == 2) {
                std::shared_ptr<Map> dudesMap = mapMain->GetPointer(nc->player->tEntity->MapID);
                Packets::SendExtAddPlayerName(concrete, nc->player->NameId, nc->player->LoginName, Entity::GetDisplayname(nc->player->tEntity->Id), dudesMap->data.Name, 0);
            }
        } else {
            if (extVersion == 2) {
                Packets::SendExtAddPlayerName(concrete, tempNameId, loginName, prettyName, clientMap->data.Name, 0);
            }
        }
    }
}