#include <System.h>
#include <Utils.h>
#include "CPE.h"

#include "Client.h"
#include "Player.h"
#include "Entity.h"
#include "Network.h"
#include "NetworkClient.h"
#include "Packets.h"
#include "Map.h"

void CPE::PreLoginExtensions(std::shared_ptr<NetworkClient> client) {
    if (GetClientExtVersion(client, CLICK_DISTANCE_EXT_NAME) == 1) {
        // -- do click distance things
        System* sm = System::GetInstance();
        Packets::SendClickDistance(client, sm->ClickDistance);
    }
    if (GetClientExtVersion(client, CUSTOM_BLOCKS_EXT_NAME) == 1) {
        Packets::SendCustomBlockSupportLevel(client, 1);
    } else {
        Client::Login(client->Id, client->player->LoginName, client->player->MPPass, client->player->ClientVersion);
    }
}

int CPE::GetClientExtVersion(std::shared_ptr<NetworkClient> client, std::string extension) {
    if (client->Extensions.find(extension) == client->Extensions.end()) {
        return 0;
    }

    return client->Extensions[extension];
}

void CPE::AfterMapActions(std::shared_ptr<NetworkClient> client) {
    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> clientMap = mm->GetPointer(client->player->tEntity->MapID);

    if (GetClientExtVersion(client, ENV_COLORS_EXT_NAME) == 1) {
        // -- Set map colors
        if (clientMap->data.ColorsSet) {
            int red = Utils::RedVal(clientMap->data.SkyColor);
            int green = Utils::GreenVal(clientMap->data.SkyColor);
            int blue = Utils::BlueVal(clientMap->data.SkyColor);
            Packets::SendSetEnvironmentColors(client, 0, red, green, blue);

            red = Utils::RedVal(clientMap->data.CloudColor);
            green = Utils::GreenVal(clientMap->data.CloudColor);
            blue = Utils::BlueVal(clientMap->data.CloudColor);
            Packets::SendSetEnvironmentColors(client, 1, red, green, blue);

            red = Utils::RedVal(clientMap->data.FogColor);
            green = Utils::GreenVal(clientMap->data.FogColor);
            blue = Utils::BlueVal(clientMap->data.FogColor);
            Packets::SendSetEnvironmentColors(client, 2, red, green, blue);

            red = Utils::RedVal(clientMap->data.alight);
            green = Utils::GreenVal(clientMap->data.alight);
            blue = Utils::BlueVal(clientMap->data.alight);
            Packets::SendSetEnvironmentColors(client, 3, red, green, blue);

            red = Utils::RedVal(clientMap->data.dlight);
            green = Utils::GreenVal(clientMap->data.dlight);
            blue = Utils::BlueVal(clientMap->data.dlight);
            Packets::SendSetEnvironmentColors(client, 4, red, green, blue);
        }
    }
    if (GetClientExtVersion(client, ENV_APPEARANCE_EXT_NAME) == 1) {
        // -- set map customization
        if (clientMap->data.CustomAppearance) {
            Packets::SendEnvMapAppearance(client, clientMap->data.CustomURL, clientMap->data.SideBlock, clientMap->data.EdgeBlock, clientMap->data.SideLevel);
        }
    }
    if (GetClientExtVersion(client, HACKCONTROL_EXT_NAME) == 1) {
        // -- Set map permissions
        Packets::SendHackControl(client, clientMap->data.Flying, clientMap->data.NoClip, clientMap->data.Speeding, clientMap->data.SpawnControl, clientMap->data.ThirdPerson, clientMap->data.JumpHeight);
    }
    if (GetClientExtVersion(client, SELECTION_CUBOID_EXT_NAME) == 1) {
        // -- Send map defined selections
    }
    if (GetClientExtVersion(client, EXT_WEATHER_CONTROL_EXT_NAME) == 1) {
        // -- Send current map weather
    }

    AfterLoginActions(client);
}

void CPE::PostEntityActions(std::shared_ptr<NetworkClient> client, std::shared_ptr<Entity> postEntity) {
    if (client->LoggedIn && GetClientExtVersion(client, CHANGE_MODEL_EXT_NAME) == 1 && postEntity->model != "Default") {
        Packets::SendChangeModel(client, postEntity->ClientId, postEntity->model);
    }
}

void CPE::AfterLoginActions(std::shared_ptr<NetworkClient> client) {
    Network* nm = Network::GetInstance();
    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Map> clientMap = mapMain->GetPointer(client->player->tEntity->MapID);
    int clientId = client->Id;
    std::string loginName = client->player->LoginName;
    std::string prettyName = Entity::GetDisplayname(client->player->tEntity->Id);
    int mapId = client->player->tEntity->MapID;
    int extVersion = CPE::GetClientExtVersion(client, EXT_PLAYER_LIST_EXT_NAME);
    int tempNameId = client->player->NameId;

    for(auto const &nc : nm->roClients) {
        if (nc->Id != clientId) {
            if (CPE::GetClientExtVersion(nc, EXT_PLAYER_LIST_EXT_NAME) == 2) {
                Packets::SendExtAddPlayerName(nc, tempNameId, loginName, prettyName, clientMap->data.Name, 0);
            }
            if (extVersion == 2) {
                std::shared_ptr<Map> dudesMap = mapMain->GetPointer(nc->player->tEntity->MapID);
                Packets::SendExtAddPlayerName(client, nc->player->NameId, nc->player->LoginName, Entity::GetDisplayname(nc->player->tEntity->Id), dudesMap->data.Name, 0);
            }
        } else {
            if (extVersion == 2) {
                Packets::SendExtAddPlayerName(client, tempNameId, loginName, prettyName, clientMap->data.Name, 0);
            }
        }
    }
}
