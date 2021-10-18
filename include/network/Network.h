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
#include <mutex>

#include "common/TaskScheduler.h"

class NetworkClient;
class ServerSocket;

class Network {
public:
    Network();
    void Start();
    void Stop();
    static Network* GetInstance();
    static Network* singleton_;
    std::shared_ptr<NetworkClient> GetClient(int id);
    std::vector<std::shared_ptr<NetworkClient>> roClients;

    int Port;
protected:
    void DeleteClient(int clientId, const std::string& message, bool sendToAll);

private:
    void UpdateNetworkStats();
    void HtmlStats();
    void NetworkEvents();
    void NetworkOutputSend();
    void NetworkOutput();
    void NetworkInput();

    std::mutex clientMutex;
    std::map<int, std::shared_ptr<NetworkClient>> _clients;
    std::unique_ptr<ServerSocket> listenSocket;
    bool isListening;
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
