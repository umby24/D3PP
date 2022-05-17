//
// Created by Wande on 2/25/2021.
//

#ifndef D3PP_CHAT_H
#define D3PP_CHAT_H
#include <iostream>
#include <string>
#include <regex>

const std::regex AllowedRegexp {R"(^\x00-\xFF)", std::regex_constants::ECMAScript };
class NetworkClient;

class Chat {
public:
    static void HandleChatEscapes(std::string &input, const int& currentEntityId);
    static std::string StringMultiline(std::string input);
    static bool StringIV(const std::string& input);
    static std::string StringGV(const std::string& input);
    
    static void NetworkSend2Player(const int& entityId, const std::string& message, std::string playerName);
    static void NetworkSend2Map(const int& entityId, const std::string& message);
    static void NetworkSend2All(const int& entityId, const std::string& message);
    static void HandleIncomingChat(const std::shared_ptr<NetworkClient>& client, const std::string& input, const char& playerId);
    static void EmoteReplace(std::string &message);
};


#endif //D3PP_CHAT_H
