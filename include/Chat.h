//
// Created by Wande on 2/25/2021.
//

#ifndef D3PP_CHAT_H
#define D3PP_CHAT_H
#include <iostream>
#include <string>
#include <regex>

const std::regex AllowedRegexp("[^A-Za-z0-9!\\^\\~$%&\\/()=?{}\t\\[\\]\\\\ ,\\\";.:\\-_#'+*<>|@\n]");
class NetworkClient;

class Chat {
public:
    static void HandleChatEscapes(std::string &input, int currentEntityId);
    static std::string StringMultiline(std::string input);
    static bool StringIV(std::string input);
    static std::string StringGV(std::string input);
    
    static void NetworkSend2Player(int entityId, std::string message, std::string playerName);
    static void NetworkSend2Map(int entityId, std::string message);
    static void NetworkSend2All(int entityId, std::string message);
    static void HandleIncomingChat(const std::shared_ptr<NetworkClient> client, std::string input, char playerId);
};


#endif //D3PP_CHAT_H
