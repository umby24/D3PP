//
// Created by Wande on 2/25/2021.
//

#include <events/EventChatAll.h>
#include <events/EventChatMap.h>
#include <events/EventChatPrivate.h>
#include "network/Chat.h"
#include "network/NetworkClient.h"
#include "network/Server.h"
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
const std::string MODULE_NAME = "CHAT";

/**
 * Escapes % -> & so users can use color codes.
 * Escapes %% -> % , so users maintain usage of % sign.
 * Inserts line breaks at <br>
 * @param input
 * @param currentEntityId
 */
void Chat::HandleChatEscapes(std::string &input) {
    Utils::replaceAll(input, "%%", "§"); // -- Temporarily move an escaped % sign to something we wont remove.
    std::string percentString = "%";
    std::string andString = "&";

    for (int i = 48; i < 57; i++) { // -- For numbers 0-9, replace %[n] with &[n]
        Utils::replaceAll(input, percentString + (char)i, andString + (char)i);
    }
    for (int i = 97; i < 102; i++) { // -- characters a-f, same thing.
        Utils::replaceAll(input, percentString + (char)i, andString + (char)i);
    }
    Utils::replaceAll(input, "§", percentString); // -- Replace the escaped percentages from before.
    Utils::replaceAll(input, "<br>", "\n"); // -- Insert line breaks [Legacy D3 thing..]
}

/**
 * Take a string and insert newlines where the text should break.
 * Should break around words and attempt to preserve color codes.
 * @param input String to be split out
 * @return A string containing LF characters to delimit newlines.
 */
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

/**
 * Determines if a given string contains invalid characters.
 * @param input String to be tested
 * @return True if invalid characters are detected.
 */
bool Chat::StringIV(const std::string& input) {
    return std::regex_match(input, AllowedRegexp);
}

/**
 * Takes an input string, substitutes system level color codes, then strips out invalid chat characters.
 */
std::string Chat::StringGV(const std::string& input) {
    std::string working(input);
    Utils::replaceAll(working, "§E", Configuration::textSettings.error);
    Utils::replaceAll(working, "§S", Configuration::textSettings.system);
    Utils::replaceAll(working, "§D", Configuration::textSettings.divider);

    return std::regex_replace(working, AllowedRegexp, "#");
}

void Chat::NetworkSend2Player(const int& entityId, const std::string& message, std::string playerName) {
    std::shared_ptr<Entity> sendingEntity = Entity::GetPointer(entityId);
    if (sendingEntity == nullptr)
        return;
    
    if (playerName.empty())
        playerName = sendingEntity->lastPrivateMessage;

    std::string output(message);

    if (!(sendingEntity->playerList != nullptr)) return;

    if (sendingEntity->playerList->MuteTime >= time(nullptr)) {
        Entity::MessageToClients(entityId, "&eYou are muted.");
        return;
    }
    HandleChatEscapes(output);
    std::shared_lock lock(D3PP::network::Server::roMutex);
    std::string message1 = "&cP " + Entity::GetDisplayname(entityId) + "&f: " + output;
    // -- Prefix all new lines with the users name.
    Utils::replaceAll(message1, "\n", "\n&cP " + Entity::GetDisplayname(entityId) + "&f: ");
    bool found = false;

    for (const auto &nc: D3PP::network::Server::roClients) {
        if (nc->GetPlayerInstance() == nullptr || nc->GetPlayerInstance()->GetEntity() == nullptr) continue;
        if (!Utils::InsensitiveCompare(nc->GetPlayerInstance()->GetEntity()->Name, playerName)) continue;

        sendingEntity->lastPrivateMessage = playerName;

        EventChatPrivate ecp;
        ecp.toEntityId = nc->GetPlayerInstance()->GetEntity()->Id;
        ecp.fromEntityId = sendingEntity->Id;
        ecp.message = output;
        Dispatcher::post(ecp);

        Logger::LogAdd("Chat",
                       sendingEntity->Name + " > " + nc->GetPlayerInstance()->GetEntity()->Name + ": " +
                       output, LogType::CHAT, GLF);
        NetworkFunctions::SystemMessageNetworkSend(nc->GetId(), message1);

        std::string message0 =
                "&c@ " + Entity::GetDisplayname(nc->GetPlayerInstance()->GetEntity()->Id) + "&f: " + output;
        Utils::replaceAll(message0, "\n", "\n&c@ " + Entity::GetDisplayname(nc->GetPlayerInstance()->GetEntity()->Id) + "&f: ");
        Entity::MessageToClients(entityId, message0);
        found = true;
        break;
    }
    if (!found) {
        Entity::MessageToClients(entityId, "&eCan't find the Player '" + playerName + "'.");
    }
}

void Chat::NetworkSend2Map(const int& entityId, const std::string& message) {
    std::shared_ptr<Entity> sendingEntity = Entity::GetPointer(entityId);

    if (sendingEntity == nullptr)
        return;

    std::string output(message);

    if (!(sendingEntity->playerList != nullptr)) return;

    if (sendingEntity->playerList->MuteTime >= time(nullptr)) {
        Entity::MessageToClients(entityId, "&eYou are muted.");
        return;
    }
    int mapId = sendingEntity->MapID;
    HandleChatEscapes(output);
    EventChatMap ecm;
    ecm.entityId = entityId;
    ecm.message = output;
    Dispatcher::post(ecm);

    if (ecm.isCancelled())
        return;

    Logger::LogAdd("Chat", sendingEntity->Name + ": " + output, LogType::CHAT, GLF);

    output = Entity::GetDisplayname(entityId) + "&f: " + output;
    // -- Prefix all new lines with the users name.
    Utils::replaceAll(output, "\n", "\n" + Entity::GetDisplayname(entityId) + "&f: ");
    NetworkFunctions::SystemMessageNetworkSend2All(mapId, output);
}

void Chat::NetworkSend2All(const int& entityId, const std::string& message) {
    std::shared_ptr<Entity> sendingEntity = Entity::GetPointer(entityId);
    if (sendingEntity == nullptr)
        return;

    std::string output(message);

    if (!(sendingEntity->playerList != nullptr)) return;

    if (sendingEntity->playerList->MuteTime >= time(nullptr)) {
        Entity::MessageToClients(entityId, "&eYou are muted.");
        return;
    }

    HandleChatEscapes(output);
    EventChatAll eca;
    eca.message = output;
    eca.entityId = entityId;
    Dispatcher::post(eca);

    if (eca.isCancelled())
        return;

    Logger::LogAdd("Chat", "# " + sendingEntity->Name + ": " + output, LogType::CHAT, GLF);

    output = "&c# " + Entity::GetDisplayname(entityId) + "&f: " + output;
    // -- Prefix all new lines with the users name.
    Utils::replaceAll(output, "\n", "\n&c# " + Entity::GetDisplayname(entityId) + "&f: ");
    NetworkFunctions::SystemMessageNetworkSend2All(-1, output);
}

void Chat::HandleIncomingChat(const std::shared_ptr<NetworkClient>& client, const std::string& input, const char& playerId) {
    std::string output(input);

    // -- Ensure our client is in a proper state.
    if (!client || !client->GetPlayerInstance() || !client->GetPlayerInstance()->GetEntity()) {
        Logger::LogAdd(MODULE_NAME, "Invalid client tried to send a chat message.", LogType::VERBOSE, GLF);
        return;
    }

    auto clientEntity = client->GetPlayerInstance()->GetEntity();

    if (CPE::GetClientExtVersion(client, LONG_MESSAGES_EXT_NAME) == 1){
        if (playerId == 1) { // -- Extend this message, don't send yet.
            clientEntity->ChatBuffer += input;
            return;
        } else { // -- Message is ready to be sent along with buffer if any.
            output = clientEntity->ChatBuffer + input;
            clientEntity->ChatBuffer.clear();
        }
    }

    CommandMain* cm = CommandMain::GetInstance();

    if (output[0] == '/') { // -- Incoming Command
        cm->CommandDo(client, output.substr(1));
    } else if (output[0] == '#') { // -- User wants to temporarily send a message outside their default preference.
        if (client->GlobalChat)
            NetworkSend2Map(clientEntity->Id, output.substr(1));
        else
            NetworkSend2All(clientEntity->Id, output.substr(1));
    } else if (output[0] == '@') { // -- Private Message
        std::vector<std::string> splitString = Utils::splitString(output);

        if (splitString.empty())
            return;

        std::string pmName = splitString[0].substr(1); // -- Trim off the '@'.
        int pmNameOffset = 2+pmName.size();

        if (pmName.empty() || output.size() < pmNameOffset) { // -- If an invalid PM name was given, just send the message out; assuming they meant to prefix with '@'.
            if (client->GlobalChat)
                NetworkSend2All(clientEntity->Id, output);
            else
                NetworkSend2Map(clientEntity->Id, output);

            return;
        }
        NetworkSend2Player(clientEntity->Id, output.substr(pmNameOffset), pmName);
    } 
    else
    {
        if (client->GlobalChat)
            NetworkSend2All(clientEntity->Id, output);
        else
            NetworkSend2Map(clientEntity->Id, output);
    }
}

void Chat::EmoteReplace(std::string& message) {
    Utils::replaceAll(message,"{:)}", "\u0001"); // ☺
    Utils::replaceAll(message,"{smile}", "\u0001");

    Utils::replaceAll(message,"{smile2}", "\u0002"); // ☻

    Utils::replaceAll(message,"{heart}", "\u0003"); // ♥
    Utils::replaceAll(message,"{hearts}", "\u0003");
    Utils::replaceAll(message,"{<3}", "\u0003");

    Utils::replaceAll(message,"{diamond}", "\u0004"); // ♦
    Utils::replaceAll(message,"{diamonds}", "\u0004");
    Utils::replaceAll(message,"{rhombus}", "\u0004");

    Utils::replaceAll(message,"{club}", "\u0005"); // ♣
    Utils::replaceAll(message,"{clubs}", "\u0005");
    Utils::replaceAll(message,"{clover}", "\u0005");
    Utils::replaceAll(message,"{shamrock}", "\u0005");

    Utils::replaceAll(message,"{spade}", "\u0006"); // ♠
    Utils::replaceAll(message,"{spades}", "\u0006");

    Utils::replaceAll(message,"{*}", "\u0007"); // •
    Utils::replaceAll(message,"{bullet}", "\u0007");
    Utils::replaceAll(message,"{dot}", "\u0007");
    Utils::replaceAll(message,"{point}", "\u0007");

    Utils::replaceAll(message,"{hole}", "\u0008"); // ◘

    Utils::replaceAll(message,"{circle}", "\u0009"); // ○
    Utils::replaceAll(message,"{o}", "\u0009");

    Utils::replaceAll(message,"{male}", "\u000B"); // ♂
    Utils::replaceAll(message,"{mars}", "\u000B");

    Utils::replaceAll(message,"{female}", "\u000C"); // ♀
    Utils::replaceAll(message,"{venus}", "\u000C");

    Utils::replaceAll(message,"{8}", "\u000D"); // ♪
    Utils::replaceAll(message,"{note}", "\u000D");
    Utils::replaceAll(message,"{quaver}", "\u000D");

    Utils::replaceAll(message,"{notes}", "\u000E"); // ♫
    Utils::replaceAll(message,"{music}", "\u000E");

    Utils::replaceAll(message,"{sun}", "\u000F"); // ☼
    Utils::replaceAll(message,"{celestia}", "\u000F");

    Utils::replaceAll(message,"{>>}", "\u0010"); // ►
    Utils::replaceAll(message,"{right2}", "\u0010");

    Utils::replaceAll(message,"{<<}", "\u0011"); // ◄
    Utils::replaceAll(message,"{left2}", "\u0011");

    Utils::replaceAll(message,"{updown}", "\u0012"); // ↕
    Utils::replaceAll(message,"{^v}", "\u0012");

    Utils::replaceAll(message,"{!!}", "\u0013"); // ‼

    Utils::replaceAll(message,"{p}", "\u0014"); // ¶
    Utils::replaceAll(message,"{para}", "\u0014");
    Utils::replaceAll(message,"{pilcrow}", "\u0014");
    Utils::replaceAll(message,"{paragraph}", "\u0014");

    Utils::replaceAll(message,"{s}", "\u0015"); // §
    Utils::replaceAll(message,"{sect}", "\u0015");
    Utils::replaceAll(message,"{section}", "\u0015");

    Utils::replaceAll(message,"{-}", "\u0016"); // ▬
    Utils::replaceAll(message,"{_}", "\u0016");
    Utils::replaceAll(message,"{bar}", "\u0016");
    Utils::replaceAll(message,"{half}", "\u0016");

    Utils::replaceAll(message,"{updown2}", "\u0017"); // ↨
    Utils::replaceAll(message,"{^v_}", "\u0017");

    Utils::replaceAll(message,"{^}", "\u0018"); // ↑
    Utils::replaceAll(message,"{up}", "\u0018");

    Utils::replaceAll(message,"{v}", "\u0019"); // ↓
    Utils::replaceAll(message,"{down}", "\u0019");

    Utils::replaceAll(message,"{>}", "\u001A"); // →
    Utils::replaceAll(message,"{->}", "\u001A");
    Utils::replaceAll(message,"{right}", "\u001A");

    Utils::replaceAll(message,"{<}", "\u001B"); // ←
    Utils::replaceAll(message,"{<-}", "\u001B");
    Utils::replaceAll(message,"{left}", "\u001B");

    Utils::replaceAll(message,"{l}", "\u001C"); // ∟
    Utils::replaceAll(message,"{angle}", "\u001C");
    Utils::replaceAll(message,"{corner}", "\u001C");

    Utils::replaceAll(message,"{<>}", "\u001D"); // ↔
    Utils::replaceAll(message,"{<->}", "\u001D");
    Utils::replaceAll(message,"{leftright}", "\u001D");

    Utils::replaceAll(message,"{^^}", "\u001E"); // ▲
    Utils::replaceAll(message,"{up2}", "\u001E");

    Utils::replaceAll(message,"{vv}", "\u001F"); // ▼
    Utils::replaceAll(message,"{down2}", "\u001F");

    Utils::replaceAll(message,"{house}", "\u007F"); // ⌂

    Utils::replaceAll(message,"{caret}", "^");
    Utils::replaceAll(message,"{hat}", "^");

    Utils::replaceAll(message,"{tilde}", "~");
    Utils::replaceAll(message,"{wave}", "~");

    Utils::replaceAll(message,"{grave}", "`");
    Utils::replaceAll(message,"{\"}", "`");
}