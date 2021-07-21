#include "CPE.h"

#include "Client.h"
#include "Player.h"
#include "Network.h"
#include "Packets.h"

void CPE::PreLoginExtensions(std::shared_ptr<NetworkClient> client) {
    if (GetClientExtVersion(client, CLICK_DISTANCE_EXT_NAME) == 1) {
        // -- do click distance things
        Packets::SendClickDistance();
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
    if (GetClientExtVersion(client, ENV_COLORS_EXT_NAME) == 1) {
        // -- Set map colors
    }
    if (GetClientExtVersion(client, ENV_APPEARANCE_EXT_NAME) == 1) {
        // -- set map customization
    }
    if (GetClientExtVersion(client, HACKCONTROL_EXT_NAME) == 1) {
        // -- Set map permissions
    }
    if (GetClientExtVersion(client, SELECTION_CUBOID_EXT_NAME) == 1) {
        // -- Send map defined selections
    }
    if (GetClientExtVersion(client, EXT_WEATHER_CONTROL_EXT_NAME) == 1) {
        // -- Send current map weather
    }
    // -- CPE_AddExtPlayer() ??
}
