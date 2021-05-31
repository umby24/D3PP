//
// Created by Wande on 2/25/2021.
//

#include "Chat.h"
const int MaxStringLength = 65;

std::string Chat::StringMultiline(std::string input) {
    std::string result;
    int maxLength = MaxStringLength;

    while (!input.empty()) {
        for(auto i = 1; i < maxLength + 1; i++) {
            if (input.substr(i, 1) == "\n") {
                result += input.substr(0, i);
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
                        result += input.substr(0, k) + "&3>>\n&3>>";
                        maxLength = 61;
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


                result += input.substr(0, i-5) + "&3>>\n&3>>";
                maxLength = 61;
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

bool Chat::StringIV(std::string input) {
    return std::regex_match(input, AllowedRegexp);
}

std::string Chat::StringGV(std::string input) {
    return std::regex_replace(input, AllowedRegexp, "#");
}

void Chat::NetworkSend2All(int entityId, std::string message) {
    std::shared_ptr<Entity> em = Entity::GetPointer(entityId);
    if (em == nullptr)
        return;

    if (em->playerList != nullptr) {
        // -- Check if muted..
        Logger::LogAdd("Chat", em->Name + ": " + message, LogType::CHAT, __FILE__, __LINE__, __FUNCTION__);
        message = Entity::GetDisplayname(entityId) + "&f: " + message;
        NetworkFunctions::SystemMessageNetworkSend2All(-1, message);
    }
}

void Chat::HandleIncomingChat(const shared_ptr<NetworkClient> client, std::string input, char playerId) {
    if (input[0] == '/') {
        // -- Do COmmands
    } else if (input[0] == '#') {
        // -- do global chat
    } else
    {
        NetworkSend2All(client->player->tEntity->Id, input);
        // -- do normal chat.
    }
    
}