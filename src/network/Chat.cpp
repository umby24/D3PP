//
// Created by Wande on 2/25/2021.
//

#include <events/EventChatAll.h>
#include <events/EventChatMap.h>
#include "network/Chat.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "world/Player.h"
#include "common/Player_List.h"
#include "world/Entity.h"
#include "network/Network_Functions.h"
#include "Command.h"

#include "common/Logger.h"
#include "Utils.h"
#include "CPE.h"
const int MaxStringLength = 65;

void Chat::HandleChatEscapes(std::string &input, int currentEntityId) {
    Utils::replaceAll(input, "%%", "§");
    std::string percentString = "%";
    std::string andString = "&";

    for (int i = 0; i < 9; i++) {
        Utils::replaceAll(input, percentString + (char)i, andString + (char)i);
    }
    for (int i = 97; i < 102; i++) {
        Utils::replaceAll(input, percentString + (char)i, andString + (char)i);
    }
    Utils::replaceAll(input, "§", percentString);
    Utils::replaceAll(input, "<br>", "\n");
    Utils::replaceAll(input, "\n", "\n" + Entity::GetDisplayname(currentEntityId) + "&f: ");
}

std::string Chat::StringMultiline(std::string input) {
    std::string result;
    int maxLength = MaxStringLength;

    while (!input.empty()) {
        for(auto i = 1; i < maxLength + 1; i++) {
            if (input.substr(i, 1) == "\n") {
                result += input.substr(0, i+1);
                input = input.substr(i+1);
                break;
            }

            if (input.substr(i, 1) == "\0") {
                result += input.substr(0, i);
                input = input.substr(i);
                break;
            }

            if (i == maxLength) {
                bool doubleBreak = false;
                for(auto k = i-5; k >= 2; --k) {
                    if (input.substr(k, 1) == " ") {
                        result += input.substr(0, k) + "&3>>\n&3>>&f";
                        maxLength = 59;
                        for (int j = k-1; j >= 1; j--) {
                            if (input.substr(j, 1) == "&") {
                                result += input.substr(j, 2);
                                maxLength -= 2;
                                break;
                            }
                        }
                        input = input.substr(k+1);
                        doubleBreak = true;
                        break;
                    }
                }
                if (doubleBreak)
                    break;


                result += input.substr(0, i-5) + "&3>>\n&3>>&f";
                maxLength = 59;
                for (int j = i-5; j >= 1; j--) {
                    if (input.substr(j, 1) == "&") {
                        result += input.substr(j, 2);
                        maxLength -= 2;
                        break;
                    }
                }
                input = input.substr(i-4);
                break;
            }
        }
    }

    return result;
}

bool Chat::StringIV(const std::string& input) {
    return std::regex_match(input, AllowedRegexp);
}

std::string Chat::StringGV(const std::string& input) {
    return std::regex_replace(input, AllowedRegexp, "#");
}

void Chat::NetworkSend2Player(int entityId, std::string message, std::string playerName) {
    std::shared_ptr<Entity> em = Entity::GetPointer(entityId);
    if (em == nullptr)
        return;
    
    if (playerName.empty())
        playerName = em->lastPrivateMessage;
    Network* nm = Network::GetInstance();

    if (em->playerList != nullptr) {
        if (em->playerList->MuteTime < time(nullptr)) {
            HandleChatEscapes(message, entityId);
            std::string message1 = "&cP " + Entity::GetDisplayname(entityId) + "&f: " + message;
            bool found = false;

            for (const auto &nc : nm->roClients) {
                if (nc->player != nullptr && nc->player->tEntity != nullptr) {
                    if (Utils::InsensitiveCompare(nc->player->tEntity->Name, playerName)) {
                        em->lastPrivateMessage = playerName;
                        Logger::LogAdd("Chat", em->Name + " > " + nc->player->tEntity->Name + ": " + message, LogType::CHAT, __FILE__, __LINE__, __FUNCTION__);
                        NetworkFunctions::SystemMessageNetworkSend(nc->GetId(), message1);

                        std::string message0 = "&c@ " + Entity::GetDisplayname(nc->player->tEntity->Id) + "&f: " +message;
                        Entity::MessageToClients(entityId, message0);
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                Entity::MessageToClients(entityId, "&eCan't find the Player '" + playerName + "'.");
            }
           
        } else {
            Entity::MessageToClients(entityId, "&eYou are muted.");
        }
    }
}

void Chat::NetworkSend2Map(int entityId, std::string message) {
    std::shared_ptr<Entity> em = Entity::GetPointer(entityId);
    if (em == nullptr)
        return;
    
    if (em->playerList != nullptr) {
        if (em->playerList->MuteTime < time(nullptr)) {
            int mapId = em->MapID;
            HandleChatEscapes(message, entityId);
            Logger::LogAdd("Chat", em->Name + ": " + message, LogType::CHAT, __FILE__, __LINE__, __FUNCTION__);

            EventChatMap ecm;
            ecm.entityId = entityId;
            ecm.message = message;
            Dispatcher::post(ecm);

            message = Entity::GetDisplayname(entityId) + "&f: " + message;
            NetworkFunctions::SystemMessageNetworkSend2All(mapId, message);
        } else {
            Entity::MessageToClients(entityId, "&eYou are muted.");
        }
    }
}

void Chat::NetworkSend2All(int entityId, std::string message) {
    std::shared_ptr<Entity> em = Entity::GetPointer(entityId);
    if (em == nullptr)
        return;

    if (em->playerList != nullptr) {
        if (em->playerList->MuteTime < time(nullptr)) {
            HandleChatEscapes(message, entityId);
            Logger::LogAdd("Chat", "# " + em->Name + ": " + message, LogType::CHAT, __FILE__, __LINE__, __FUNCTION__);

            EventChatAll eca;
            eca.message = message;
            eca.entityId = entityId;
            Dispatcher::post(eca);

            message = "&c# " + Entity::GetDisplayname(entityId) + "&f: " + message;
            NetworkFunctions::SystemMessageNetworkSend2All(-1, message);
        } else {
            Entity::MessageToClients(entityId, "&eYou are muted.");
        }
    }
}

void Chat::HandleIncomingChat(const std::shared_ptr<NetworkClient>& client, std::string input, char playerId) {
    if (CPE::GetClientExtVersion(client, LONG_MESSAGES_EXT_NAME) == 1){
        if (playerId == 1) {
            client->player->tEntity->ChatBuffer += input;
            return;
        } else {
            input = client->player->tEntity->ChatBuffer + input;
            client->player->tEntity->ChatBuffer = "";
        }
    }
    CommandMain* cm = CommandMain::GetInstance();

    if (input[0] == '/') {
        cm->CommandDo(client, input.substr(1));
    } else if (input[0] == '#') {
        if (client->GlobalChat)
            NetworkSend2Map(client->player->tEntity->Id, input.substr(1));
        else
            NetworkSend2All(client->player->tEntity->Id, input.substr(1));
        // -- do global chat
    } else if (input[0] == '@') {
        std::vector<std::string> splitString = Utils::splitString(input);
        std::string pmName = splitString[0].substr(1); // -- Trim off the '@'.
        NetworkSend2Player(client->player->tEntity->Id, input.substr(2+pmName.size()), pmName);
    } 
    else
    {
        if (client->GlobalChat)
            NetworkSend2All(client->player->tEntity->Id, input);
        else
            NetworkSend2Map(client->player->tEntity->Id, input);
    }
    
}