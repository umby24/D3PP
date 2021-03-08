//
// Created by unknown on 2/18/2021.
//

#ifndef D3PP_NETWORK_H
#define D3PP_NETWORK_H

#define NETWORK_TEMP_BUFFER_SIZE 2000
#define NETWORK_BUFFER_SIZE 3000000
#define NETWORK_PACKET_SIZE 1400
#define NETWORK_CLIENT_TIMEOUT 6000*5

#include <string>
#include <map>
#include <vector>
#include <thread>

#ifndef __linux__
#include "network/WindowsServerSockets.h"
#include "network/WindowsSockets.h"
#else
#include "network/LinuxServerSockets.h"
#endif

#include "Mem.h"
#include "TaskScheduler.h"
#include "watchdog.h"
#include "json.hpp"
using json = nlohmann::json;

class NetworkClient {
public:
    NetworkClient();
    NetworkClient(const Sockets& socket);
    void InputWriteBuffer(char* data, int size);
    void OutputReadBuffer(char* dataBuffer, int size);
    void OutputAddOffset(int bytes);
    void OutputPing();
    void Kick(std::string message, bool hide);
    int Id;
    std::string IP;
    char* InputBuffer;
    int InputBufferOffset;
    int InputBufferAvailable;
    char* OutputBuffer;
    int OutputBufferOffset;
    int OutputBufferAvailable;
    int DisconnectTime;
    int LastTimeEvent;
    int UploadRate;
    int DownloadRate;
    int UploadRateCounter;
    int DownloadRateCounter;
    int Ping;
    int PingSentTime;
    int PingTime;
    bool LoggedIn;
    bool CPE;
    int CustomBlocksLevel;
    bool GlobalChat;
    Sockets clientSocket;
    std::map<std::string, int> Extensions;
    std::vector<bool> Selections;
private:
    void OutputWriteByte(char value);
    void OutputWriteShort(short value);
    void OutputWriteInt(int value);
    void OutputWriteString(std::string value);
    void OutputWriteBlob(char* data, int dataSize);
};

class Network {
public:
    Network();
    void Save();
    void Load();
    void Start();
    void Stop();
protected:
    void AddClient(Sockets clientSocket);
    void DeleteClient(int clientId, std::string message, bool sendToAll);
    NetworkClient* GetClient(int id);
private:
    void UpdateNetworkStats();
    void HtmlStats();
    void MainFunc();
    void ClientAcceptance();
    void NetworkEvents();
    void NetworkOutputSend();
    void NetworkOutput();
    void NetworkInput();

    ServerSocket listenSocket;
    bool isListening;
    char* TempBuffer;
    int TimerRate;
    int UploadRate;
    int DownloadRate;
    int UploadRateCounter;
    int DownloadRateCounter;
    int Port;
    std::map<int, NetworkClient> _clients;
    time_t lastModifiedTime;
    bool SaveFile;
    std::thread _acceptThread;
};

const std::string NETWORK_HTML = R"(<html>
  <head>
    <title>Minecraft-Server Network</title>
  </head>
  <body>
      <b><u>Overview:</u></b><br>
      Port: 13337.<br>
      Download_Rate: 0.000kbytes/s.<br>
      Download_Rate: <font color="#FF0000"></font><br>
      Upload_Rate: 0.000kbytes/s.<br>
      Upload_Rate: <font color="#FF0000"></font><br>
      <br>
      <br>
      <br>
      <b><u>Clients:</u></b><br>
      <br>
      <table border=1>        <tr>
          <th><b>ID</b></th>
          <th><b>Login_Name</b></th>
          <th><b>Client_Version</b></th>
          <th><b>IP</b></th>
          <th><b>Download_Rate</b></th>
          <th><b>Upload_Rate</b></th>
          <th><b>Entity_ID</b></th>
        </tr>
      </table>      <br>
      <br>
      <br>
      Site generated in 0 ms. 19:35:33  12.02.2021 (1613158533)<br>
  </body>
</html>)";

#endif //D3PP_NETWORK_H
