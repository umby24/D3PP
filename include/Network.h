//
// Created by unknown on 2/18/2021.
//

#ifndef D3PP_NETWORK_H
#define D3PP_NETWORK_H

#define NETWORK_TEMP_BUFFER_SIZE 2000
#define NETWORK_BUFFER_SIZE 3000000
#define NETWORK_PACKET_SIZE 1400
#define NETWORK_CLIENT_TIMEOUT 30

#include <string>
#include <map>
#include <vector>
#include <thread>
#include <memory>

#include "TaskScheduler.h"

#include "json.hpp"


using json = nlohmann::json;

class Player;
class Sockets;
class ServerSocket;

class NetworkClient {
public:
    NetworkClient();
    NetworkClient(std::unique_ptr<Sockets> socket);
    // -- Input Buffer Commands
    void InputAddOffset(int bytes);
    char InputReadByte();
    short InputReadShort();
    int InputReadInt();
    std::string InputReadString();
    void InputReadBytes(char* data, int datalen);

    void InputWriteBuffer(char* data, int size);
    // -- Output Buffer Commands
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
    int CustomExtensions;
    int CustomBlocksLevel;
    bool GlobalChat;
    std::unique_ptr<Sockets> clientSocket;
    std::unique_ptr<Player> player;

    std::map<std::string, int> Extensions;
    std::vector<bool> Selections;
    void OutputWriteByte(char value);
    void OutputWriteShort(short value);
    void OutputWriteInt(int value);
    void OutputWriteString(std::string value);
    void OutputWriteBlob(const char* data, int dataSize);
};

class Network {
public:
    Network();
    void Save();
    void Load();
    void Start();
    void Stop();
    static Network* GetInstance();
    static Network* singleton_;
    std::shared_ptr<NetworkClient> GetClient(int id);
    std::map<int, std::shared_ptr<NetworkClient>> _clients;

    int Port;
protected:
    void DeleteClient(int clientId, std::string message, bool sendToAll);

private:
    void UpdateNetworkStats();
    void HtmlStats();
    void MainFunc();
    void NetworkEvents();
    void NetworkOutputSend();
    void NetworkOutput();
    void NetworkInput();

    std::unique_ptr<ServerSocket> listenSocket;
    bool isListening;
    char* TempBuffer;
    int TimerRate;
    int UploadRate;
    int DownloadRate;
    int UploadRateCounter;
    int DownloadRateCounter;


    time_t lastModifiedTime;
    bool SaveFile;
};
const std::string NETWORK_HTML_FILENAME = "Network_HTML";

const std::string NETWORK_HTML = R"(<html>
  <head>
    <title>Minecraft-Server Network</title>
  </head>
  <body>
      <b><u>Overview:</u></b><br>
      Port: [PORT].<br>
      Download_Rate: [DRATE].<br>
      Upload_Rate: [URATE].<br>
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
        [CLIENTS_TABLE]
      </table>      <br>
      <br>
      <br>
      Site generated in [GEN_TIME] ms at [GEN_TIMESTAMP]<br>
  </body>
</html>)";

#endif //D3PP_NETWORK_H
