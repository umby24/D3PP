#include "CPE.h"

#include "Client.h"
#include "Player.h"
#include "Network.h"
#include "Packets.h"

void CPE::PreLoginExtensions(std::shared_ptr<NetworkClient> client) {
    if (client->Extensions.find("ClickDistance") != client->Extensions.end()) {
        // -- do click distance things
    }
    if (client->Extensions.find("CustomBlocks") != client->Extensions.end()) {
        if (client->Extensions["CustomBlocks"] == 1) {
            Packets::SendCustomBlockSupportLevel(client, 1);
        }
    } else {
        Client::Login(client->Id, client->player->LoginName, client->player->MPPass, client->player->ClientVersion);
    }
}