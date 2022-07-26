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

namespace D3PP::world {
    class IMinecraftPlayer;
}

class ConsoleClient : public IMinecraftClient {
public:

    static std::shared_ptr<ConsoleClient> GetInstance() { if (instance == nullptr) instance = std::make_shared<ConsoleClient>(); return instance; }

    int GetId() override { return -200; };
    int GetRank() override { return 32767; };
    int GetCustomBlocksLevel() override { return 0; }
    int GetPing() override { return 1337; }
    int GetMapId() override { return -1; }
    std::string GetLoginName() override { return "[CONSOLE]"; }
    bool GetGlobalChat() override { return true; }
    void SetGlobalChat(bool active) override { /* noop */ }
    bool IsStopped() override { return false; }
    void SendChat(std::string message) override {
        Utils::replaceAll(message, "<br>", "\n");
        Logger::LogAdd("CONSOLE", message, LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    }
    void Kick(const std::string& message, bool hide) override { /* noop */ }
    void SendDefineBlock(BlockDefinition newBlock) override;
    void SendDeleteBlock(unsigned char blockId) override { }
    void SpawnEntity(std::shared_ptr<Entity> e) override { }
    void DespawnEntity(std::shared_ptr<Entity> e) override { }
    bool IsDataAvailable() override { return false; }
    void SendQueued() override { }
    void HandleData() override { }
    void SendPacket(D3PP::network::IPacket& p) override { }
    bool GetLoggedIn() override { return true; }
    void NotifyDataAvailable() override {}
        void Undo(int steps) override {}
    void Redo(int steps) override {}
    void AddUndoItem(const D3PP::Common::UndoItem& item) override {}

    std::shared_ptr<D3PP::world::IMinecraftPlayer> GetPlayerInstance() override { return nullptr; }
private:
    static std::shared_ptr<ConsoleClient> instance;
};

#endif //D3PP_CONSOLECLIENT_H
