//
// Created by Wande on 2/25/2021.
//

#include <events/EventChatAll.h>
#include <events/EventChatMap.h>
#include "network/Chat.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "common/Configuration.h"
#include "world/Player.h"
#include "common/Player_List.h"
#include "world/Entity.h"
#include "network/Network_Functions.h"
#include "Command.h"

#include "common/Logger.h"
#include "Utils.h"
#include "CPE.h"
const int MaxStringLength = 65;

void Chat::HandleChatEscapes(std::string &input, const int& currentEntityId) {
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
    std::string working(input);
    Utils::replaceAll(working, "§E", Configuration::textSettings.error);
    Utils::replaceAll(working, "§S", Configuration::textSettings.system);
    Utils::replaceAll(working, "§D", Configuration::textSettings.divider);

    return std::regex_replace(working, AllowedRegexp, "#");
}

void Chat::NetworkSend2Player(const int& entityId, const std::string& message, std::string playerName) {
    std::shared_ptr<Entity> em = Entity::GetPointer(entityId);
    if (em == nullptr)
        return;
    
    if (playerName.empty())
        playerName = em->lastPrivateMessage;

    std::string output(message);
    Network* nm = Network::GetInstance();

    if (em->playerList != nullptr) {
        if (em->playerList->MuteTime < time(nullptr)) {
            HandleChatEscapes(output, entityId);
            std::string message1 = "&cP " + Entity::GetDisplayname(entityId) + "&f: " + output;
            bool found = false;

            for (const auto &nc : nm->roClients) {
                if (nc->player != nullptr && nc->player->tEntity != nullptr) {
                    if (Utils::InsensitiveCompare(nc->player->tEntity->Name, playerName)) {
                        em->lastPrivateMessage = playerName;
                        Logger::LogAdd("Chat", em->Name + " > " + nc->player->tEntity->Name + ": " + output, LogType::CHAT, __FILE__, __LINE__, __FUNCTION__);
                        NetworkFunctions::SystemMessageNetworkSend(nc->GetId(), message1);

                        std::string message0 = "&c@ " + Entity::GetDisplayname(nc->player->tEntity->Id) + "&f: " +output;
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

void Chat::NetworkSend2Map(const int& entityId, const std::string& message) {
    std::shared_ptr<Entity> em = Entity::GetPointer(entityId);
    if (em == nullptr)
        return;
    std::string output(message);
    if (em->playerList != nullptr) {
        if (em->playerList->MuteTime < time(nullptr)) {
            int mapId = em->MapID;
            HandleChatEscapes(output, entityId);
            Logger::LogAdd("Chat", em->Name + ": " + output, LogType::CHAT, __FILE__, __LINE__, __FUNCTION__);

            EventChatMap ecm;
            ecm.entityId = entityId;
            ecm.message = output;
            Dispatcher::post(ecm);

            output = Entity::GetDisplayname(entityId) + "&f: " + output;
            NetworkFunctions::SystemMessageNetworkSend2All(mapId, output);
        } else {
            Entity::MessageToClients(entityId, "&eYou are muted.");
        }
    }
}

void Chat::NetworkSend2All(const int& entityId, const std::string& message) {
    std::shared_ptr<Entity> em = Entity::GetPointer(entityId);
    if (em == nullptr)
        return;

    std::string output(message);

    if (em->playerList != nullptr) {
        if (em->playerList->MuteTime < time(nullptr)) {
            HandleChatEscapes(output, entityId);
            Logger::LogAdd("Chat", "# " + em->Name + ": " + output, LogType::CHAT, __FILE__, __LINE__, __FUNCTION__);

            EventChatAll eca;
            eca.message = output;
            eca.entityId = entityId;
            Dispatcher::post(eca);

            output = "&c# " + Entity::GetDisplayname(entityId) + "&f: " + output;
            NetworkFunctions::SystemMessageNetworkSend2All(-1, output);
        } else {
            Entity::MessageToClients(entityId, "&eYou are muted.");
        }
    }
}

void Chat::HandleIncomingChat(const std::shared_ptr<NetworkClient>& client, const std::string& input, const char& playerId) {
    std::string output(input);
    if (CPE::GetClientExtVersion(client, LONG_MESSAGES_EXT_NAME) == 1){
        if (playerId == 1) {
            client->player->tEntity->ChatBuffer += input;
            return;
        } else {
            output = client->player->tEntity->ChatBuffer + input;
            client->player->tEntity->ChatBuffer = "";
        }
    }
    CommandMain* cm = CommandMain::GetInstance();

    if (output[0] == '/') {
        cm->CommandDo(client, output.substr(1));
    } else if (output[0] == '#') {
        if (client->GlobalChat)
            NetworkSend2Map(client->player->tEntity->Id, output.substr(1));
        else
            NetworkSend2All(client->player->tEntity->Id, output.substr(1));
        // -- do global chat
    } else if (output[0] == '@') {
        std::vector<std::string> splitString = Utils::splitString(output);
        std::string pmName = splitString[0].substr(1); // -- Trim off the '@'.
        NetworkSend2Player(client->player->tEntity->Id, output.substr(2+pmName.size()), pmName);
    } 
    else
    {
        if (client->GlobalChat)
            NetworkSend2All(client->player->tEntity->Id, output);
        else
            NetworkSend2Map(client->player->tEntity->Id, output);
    }
    
}