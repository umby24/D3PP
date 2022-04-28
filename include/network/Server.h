//
// Created by Wande on 1/28/2022.
//

#ifndef D3PP_SERVER_H
#define D3PP_SERVER_H
#include <thread>
#include <memory>
#include <map>
#include <vector>
#include <string>
#include <mutex>

#include "common/TaskScheduler.h"

class ServerSocket;
class IMinecraftClient;


namespace D3PP::network {
    class IPacket;

 class Server : public TaskItem {
 public:
     static int OnlinePlayers;
     static float BytesSent;
     static float BytesReceived;
     static std::atomic<int> SentIncrement;
     static std::atomic<int> ReceivedIncrement;
     static std::vector<std::shared_ptr<IMinecraftClient>> roClients;

     Server();
     void Shutdown();
     
     static void Start();
     static void Stop();

     static void RegisterClient(const std::shared_ptr<IMinecraftClient>& client);
     static void UnregisterClient(const std::shared_ptr<IMinecraftClient>& client);
     static void SendToAll(IPacket& packet, std::string extension, int extVersion);
     static void SendAllExcept(IPacket& packet, std::shared_ptr<IMinecraftClient> toNot);
 private:
     static Server* m_Instance;
     static std::map<int, std::shared_ptr<IMinecraftClient>> m_clients;
     std::unique_ptr<ServerSocket> m_serverSocket;
     std::thread m_handleThread;
     static std::mutex m_ClientMutex;
     int m_Port;

     void HandleClientData();
     void HandleIncomingClient();
     void MainFunc();
     void HandleEvents();
     static void RebuildRoClients();
 };
}
#endif //D3PP_SERVER_H
