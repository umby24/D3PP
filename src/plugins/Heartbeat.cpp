//
// Created by unknown on 6/29/21.
//

#include "plugins/Heartbeat.h"

#include <algorithm>
#include "System.h"
#include "network/Network.h"
#include "network/Server.h"
#include "world/Player.h"
#include "network/httplib.h"
#include "digestpp/digestpp.hpp"
#include "Utils.h"
#include "common/Logger.h"
#include "common/Configuration.h"
#include "network/NetworkClient.h"
#include "json.hpp"
using json = nlohmann::json;

const std::string MODULE_NAME = "Heartbeat";
Heartbeat* Heartbeat::Instance = nullptr;

void Heartbeat::Beat() {
    httplib::Client cli(CLASSICUBE_NET_URL);

    httplib::Params params;
    params.emplace("name", Configuration::GenSettings.name);
    params.emplace("port", stringulate(Configuration::NetSettings.ListenPort));
    params.emplace("users", stringulate(GetUniqueOnlinePlayers()));
    params.emplace("max", stringulate(Configuration::NetSettings.MaxPlayers));
    params.emplace("public", Configuration::NetSettings.Public ? "true" : "false");
    params.emplace("version", "7");
    params.emplace("salt", salt);
    params.emplace("software", "&e" + System::ServerName);

    auto res = cli.Post(CLASSICUBE_HEARTBEAT_PATH, params);
    if (!res) {
        Logger::LogAdd(MODULE_NAME, "Heartbeat failed.", L_ERROR, __FILE__, __LINE__, __FUNCTION__);
    }
    else if (res->status != 200 || (res->body.find("http") == std::string::npos)) {
        Logger::LogAdd(MODULE_NAME, "Heartbeat failed.", L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        try {
            json j = json::parse(res->body);
            std::string reasonStr;
            for (auto const &f: j["errors"]) {
                if (f.is_string())
                    reasonStr += f.get<std::string>() + ", ";
                if (f.is_array()) {
                    for (auto const &q: f) {
                        reasonStr += q.get<std::string>() + ", ";
                    }
                }
            }

            Logger::LogAdd(MODULE_NAME, reasonStr, L_ERROR, GLF);
        } catch(std::exception e) {
            Logger::LogAdd(MODULE_NAME, "Error parsing Heartbeat. Response: " + res->body, L_ERROR, GLF);
        }
    } else {
        if (isFirstBeat) {
            Logger::LogAdd(MODULE_NAME, "Heartbeat sent.", NORMAL, __FILE__, __LINE__, __FUNCTION__);
            serverUrl = res->body;
            Logger::LogAdd(MODULE_NAME, "Heartbeat URL: " + serverUrl, NORMAL, GLF);
            isFirstBeat = false;
        }
    }

    lastBeat = time(nullptr);
}

Heartbeat::Heartbeat() {
    salt = "";
    isPublic = false;
    lastBeat = time(nullptr)-25;
    isFirstBeat = true;
    
    this->Setup = [this] { Init(); };
    this->Main = [this] { MainFunc(); };
    this->Interval = std::chrono::seconds(1);
    this->LastRun = std::chrono::system_clock::now();
    TaskScheduler::RegisterTask(MODULE_NAME, *this);
}

std::string Heartbeat::CreateSalt() {
    std::string result;

    for (auto i = 0; i < 33; i++) {
        result += (char)(65 + Utils::RandomNumber(25));
    }

    return result;
}

void Heartbeat::Init() {
    salt = Configuration::NetSettings.Salt;
    if (salt.empty()) {
        salt = CreateSalt();
        Configuration::NetSettings.Salt = salt;
        Configuration::GetInstance()->Save();
    }
}

void Heartbeat::MainFunc() {
    if (time(nullptr) - lastBeat > 30) { // -- Heartbeat every 30 seconds.
        Beat();
    }
}

Heartbeat *Heartbeat::GetInstance() {
    if (Instance == nullptr) {
        Instance = new Heartbeat();
    }

    return Instance;
}

bool Heartbeat::VerifyName(std::string name, std::string pass) {
    std::string toHash = salt + name;
    std::string valid = digestpp::md5().absorb(toHash).hexdigest();

    return Utils::InsensitiveCompare(valid, pass);
}

int Heartbeat::GetUniqueOnlinePlayers() {
    int result = 0;
    std::vector<std::string> uniqueNames;
    std::shared_lock sharedLock(D3PP::network::Server::roMutex);
    for (auto const &nc: D3PP::network::Server::roClients) {
        if (!nc->GetLoggedIn())
            continue;

        if (std::find(uniqueNames.begin(), uniqueNames.end(), nc->GetLoginName()) != uniqueNames.end()) {
            continue;
        }

        uniqueNames.push_back(nc->GetLoginName());
        result++;
    }

    return result;
}
