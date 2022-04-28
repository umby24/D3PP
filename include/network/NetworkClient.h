//
// Created by unknown on 8/24/21.
//

#ifndef D3PP_NETWORKCLIENT_H
#define D3PP_NETWORKCLIENT_H
#define MAX_SELECTION_BOXES 255
#define CONSOLE_CLIENT_ID -200
#include <string>
#include <memory>
#include <mutex>
#include <map>
#include <vector>
#include <atomic>

class Sockets;
class ByteBuffer;
class Entity;
class Event;
struct BlockDefinition;

namespace D3PP::network {
    class IPacket;
}

namespace D3PP::Common {
    struct UndoItem;
    struct Vector3S;
}

namespace D3PP::world {
    class IMinecraftPlayer;
}

class IMinecraftClient {
public:
    IMinecraftClient()= default;;
    virtual ~IMinecraftClient()= default;;
    virtual int GetId() = 0;
    virtual int GetRank() = 0;
    virtual int GetCustomBlocksLevel() = 0;
    virtual int GetPing() = 0;
    virtual std::string GetLoginName() = 0;
    virtual bool GetGlobalChat() = 0;
    virtual int GetMapId() = 0;
    virtual bool GetLoggedIn() = 0;
    virtual void SetGlobalChat(bool active) = 0;
    virtual bool IsStopped() = 0;
    virtual void SendChat(std::string message) = 0;
    virtual void Kick(const std::string& message, bool hide) = 0;
    virtual void SendDefineBlock(BlockDefinition newBlock) = 0;
    virtual void SendDeleteBlock(unsigned char blockId) = 0;
    virtual void SpawnEntity(std::shared_ptr<Entity> e) = 0;
    virtual void DespawnEntity(std::shared_ptr<Entity> e) = 0;
    virtual bool IsDataAvailable() = 0;
    virtual void NotifyDataAvailable() = 0;
    virtual void SendQueued() = 0;
    virtual void HandleData() = 0;
    virtual void SendPacket(D3PP::network::IPacket& p) = 0;
    virtual void Undo(int steps) = 0;
    virtual void Redo(int steps) = 0;
    virtual void AddUndoItem(const D3PP::Common::UndoItem& item) = 0;
    virtual std::shared_ptr<D3PP::world::IMinecraftPlayer> GetPlayerInstance() = 0;
};

class NetworkClient : public IMinecraftClient {
    friend class Client;
public:
    NetworkClient();
    ~NetworkClient() override;
    explicit NetworkClient(std::unique_ptr<Sockets> socket);
    NetworkClient(NetworkClient &client);

    // -- Output Buffer Commands
    void Kick(const std::string& message, bool hide) override;
    void Shutdown(const std::string& reason);

    bool canSend;
    bool canReceive;
    std::shared_ptr<ByteBuffer> SendBuffer;
    std::shared_ptr<ByteBuffer> ReceiveBuffer;
    std::mutex sendLock;

    std::string IP;

    float Ping;
    short PingVal;
    std::chrono::time_point<std::chrono::steady_clock> PingSentTime;
    std::chrono::time_point<std::chrono::steady_clock> PingTime;
    bool LoggedIn;
    bool CPE;
    int CustomExtensions;
    int CustomBlocksLevel;
    bool GlobalChat;

    std::map<std::string, int> Extensions;

    // -- Not overrides
    void HoldThis(unsigned char blockType, bool canChange);
    void CreateSelection(unsigned char selectionId, std::string label, D3PP::Common::Vector3S start, D3PP::Common::Vector3S end, D3PP::Common::Vector3S color, short opacity);
    void DeleteSelection(unsigned char selectionId);
    void SetWeather(int weatherType);
    void SendHackControl(bool canFly, bool noclip, bool speeding, bool spawnControl, bool thirdperson, int jumpHeight);
    void SetBlockPermissions(int blockId, bool canPlace, bool canDelete);

    // -- Overrides
    int GetId() override { return Id; };
    int GetRank() override;
    int GetCustomBlocksLevel() override { return CustomBlocksLevel; }
    bool IsStopped() override;
    int GetPing() override;
    std::string GetLoginName() override;
    bool GetGlobalChat() override;
    int GetMapId() override;
    bool GetLoggedIn() override;
    void SetGlobalChat(bool active) override;

    void SpawnEntity(std::shared_ptr<Entity> e) override;
    void DespawnEntity(std::shared_ptr<Entity> e) override;
    void SendChat(std::string message) override;


    void SendDefineBlock(BlockDefinition newBlock) override;
    void SendDeleteBlock(unsigned char blockId) override;

    bool IsDataAvailable() override;
    void SendQueued() override;
    void HandleData() override;
    void SendPacket(D3PP::network::IPacket& p) override;
    void Undo(int steps) override;
    void Redo(int steps) override;
    void AddUndoItem(const D3PP::Common::UndoItem& item) override;
    void NotifyDataAvailable() override;
    std::shared_ptr<D3PP::world::IMinecraftPlayer> GetPlayerInstance() override;
    std::shared_ptr<NetworkClient> GetSelfPointer() const;

private:
    int Id;
    int eventSubId, addSubId, removeSubId, m_currentUndoIndex;
    std::vector<D3PP::Common::UndoItem> m_undoItems;
    std::atomic<bool> DataAvailable;
    std::atomic<bool> DataWaiting;
    std::unique_ptr<Sockets> clientSocket;
    std::shared_ptr<D3PP::world::IMinecraftPlayer> player;
    std::vector<unsigned char> Selections;
    time_t DisconnectTime;
    time_t LastTimeEvent;

    void SubEvents();
    void HandleEvent(const Event &event);
    bool ReadData();
    void MainFunc();
    void DataReady();
    void OutputPing();
};
#endif //D3PP_NETWORKCLIENT_H
