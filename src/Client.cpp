//
// Created by Wande on 3/31/2021.
//

#include "Client.h"

void Client::Login(int clientId, std::string name, std::string mppass, char version) {
    Network *n = Network::GetInstance();
    PlayerMain *pm = PlayerMain::GetInstance();
    Player_List *pl = Player_List::GetInstance();

    shared_ptr<NetworkClient> c = n->GetClient(clientId);

    c->player = make_unique<Player>();
    c->player->LoginName = name;
    c->player->MPPass = mppass;
    c->player->ClientVersion = version;

    bool preLoginCorrect = true;
    if (version != 7) {

    } else if (Chat::StringIV(name)) {

    } else if (name.empty()) {

    }

    if (!preLoginCorrect) {
        return;
    }
    PlayerListEntry *entry = pl->GetPointer(c->player->LoginName);

    if (entry == nullptr) {
        pl->Add(c->player->LoginName);
    } else {
        if (entry->Banned) {
            c->Kick("You are banned", true);
            return;
        }
        entry->Online = 1;
        entry->LoginCounter++;
        entry->IP = c->IP;
        c->GlobalChat = entry->GlobalChat;
    }
}
