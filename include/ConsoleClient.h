//
// Created by unknown on 10/28/21.
//

#ifndef D3PP_CONSOLECLIENT_H
#define D3PP_CONSOLECLIENT_H

#include "common/Logger.h"
#include "network/NetworkClient.h"
#include "Utils.h"

class IMinecraftClient;
struct BlockDefinition;

class ConsoleClient : public IMinecraftClient {
public:

    static std::shared_ptr<ConsoleClient> GetInstance() { if (instance == nullptr) instance = std::make_shared<ConsoleClient>(); return instance; }

    int GetId() override { return -200; };
    int GetRank() override { return 32767; };
    int GetCustomBlocksLevel() override { return 0; }
    int GetPing() override { return 1337; }
    std::string GetLoginName() { return "[CONSOLE]"; }
    bool GetGlobalChat() { return true; }
    void SetGlobalChat(bool active) { /* noop */ }
    bool IsStopped() { return false; }
    void SendChat(std::string message) {
        Utils::replaceAll(message, "<br>", "\n");
        Logger::LogAdd("CONSOLE", message, LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    }
    void Kick(const std::string& message, bool hide) { /* noop */ }
    void SendDefineBlock(BlockDefinition newBlock) override;
    void SendDeleteBlock(unsigned char blockId) override { }
    void SpawnEntity(std::shared_ptr<Entity> e) override { }
    void DespawnEntity(std::shared_ptr<Entity> e) override { }
private:
    static std::shared_ptr<ConsoleClient> instance;
};

#endif //D3PP_CONSOLECLIENT_H
