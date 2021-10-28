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
class Player;
class Entity;
class Event;

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
    virtual void SetGlobalChat(bool active) = 0;
    virtual bool IsStopped() = 0;
    virtual void SendChat(std::string message) = 0;
    virtual void Kick(const std::string& message, bool hide) = 0;
};

class NetworkClient : public IMinecraftClient {
public:
    NetworkClient();
    ~NetworkClient() override;
    explicit NetworkClient(std::unique_ptr<Sockets> socket);
    NetworkClient(NetworkClient &client);

    void DataReady();
    // -- Output Buffer Commands
    void OutputPing();
    void Kick(const std::string& message, bool hide) override;
    bool canSend;
    bool canReceive;
    std::unique_ptr<ByteBuffer> SendBuffer;
    std::unique_ptr<ByteBuffer> ReceiveBuffer;
    std::mutex sendLock;

    std::string IP;
    unsigned char lastPacket;
    time_t DisconnectTime;
    time_t LastTimeEvent;
    int UploadRate;
    int DownloadRate;
    int UploadRateCounter;
    int DownloadRateCounter;
    float Ping;
    short PingVal;
    std::chrono::time_point<std::chrono::steady_clock> PingSentTime;
    std::chrono::time_point<std::chrono::steady_clock> PingTime;
    bool LoggedIn;
    bool CPE;
    int CustomExtensions;
    int CustomBlocksLevel;
    bool GlobalChat;
    std::atomic<bool> DataAvailable;
    std::unique_ptr<Sockets> clientSocket;
    std::unique_ptr<Player> player;

    std::map<std::string, int> Extensions;
    std::vector<unsigned char> Selections;

    int GetId() override { return Id; };
    int GetRank() override;
    int GetCustomBlocksLevel() override { return CustomBlocksLevel; }
    bool IsStopped() override;
    int GetPing() override;
    std::string GetLoginName() override;
    bool GetGlobalChat() override;
    void SetGlobalChat(bool active) override;

    void SpawnEntity(std::shared_ptr<Entity> e);
    void DespawnEntity(std::shared_ptr<Entity> e);
    void SendChat(std::string message) override;
    void HoldThis(unsigned char blockType, bool canChange);
    void CreateSelection(unsigned char selectionId, std::string label, short startX, short startY, short startZ, short endX, short endY, short endZ, short red, short green, short blue, short opacity);
    void DeleteSelection(unsigned char selectionId);
    void SetWeather(int weatherType);
    void SendHackControl(bool canFly, bool noclip, bool speeding, bool spawnControl, bool thirdperson, int jumpHeight);
    void SetBlockPermissions(int blockId, bool canPlace, bool canDelete);
private:
    int Id;
    int eventSubId;
    std::shared_ptr<NetworkClient> GetSelfPointer();
    void SubEvents();
    void HandleEvent(const Event &event);
};
#endif //D3PP_NETWORKCLIENT_H
