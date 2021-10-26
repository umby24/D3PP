//
// Created by unknown on 8/24/21.
//

#ifndef D3PP_NETWORKCLIENT_H
#define D3PP_NETWORKCLIENT_H
#define MAX_SELECTION_BOXES 255

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

class NetworkClient {
public:
    NetworkClient();
    ~NetworkClient();
    explicit NetworkClient(std::unique_ptr<Sockets> socket);
    NetworkClient(NetworkClient &client);

    void DataReady();
    // -- Output Buffer Commands
    void OutputPing();
    void Kick(const std::string& message, bool hide);
    bool canSend;
    bool canReceive;
    std::unique_ptr<ByteBuffer> SendBuffer;
    std::unique_ptr<ByteBuffer> ReceiveBuffer;
    std::mutex sendLock;

    int Id;
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

    void SpawnEntity(std::shared_ptr<Entity> e);
    void DespawnEntity(std::shared_ptr<Entity> e);

    void HoldThis(unsigned char blockType, bool canChange) const;
    void CreateSelection(unsigned char selectionId, std::string label, short startX, short startY, short startZ, short endX, short endY, short endZ, short red, short green, short blue, short opacity);
    void DeleteSelection(unsigned char selectionId);
    void SetWeather(int weatherType);
    void SendHackControl(bool canFly, bool noclip, bool speeding, bool spawnControl, bool thirdperson, int jumpHeight);
    void SetBlockPermissions(int blockId, bool canPlace, bool canDelete);
private:
    int eventSubId;
    void SubEvents();
    void HandleEvent(const Event &event);
};
#endif //D3PP_NETWORKCLIENT_H
