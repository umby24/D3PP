//
// Created by unknown on 6/29/21.
//

#include "plugins/Heartbeat.h"


#include "System.h"
#include "common/TaskScheduler.h"
#include "network/Network.h"
#include "world/Player.h"
#include "network/httplib.h"
#include "digestpp/digestpp.hpp"
#include "Utils.h"
#include "common/Logger.h"
#include "common/Configuration.h"
#include "json.hpp"
using json = nlohmann::json;

const std::string MODULE_NAME = "Heartbeat";
Heartbeat* Heartbeat::Instance = nullptr;

void Heartbeat::Beat() {
    Network* nMain = Network::GetInstance();
    PlayerMain* pMain = PlayerMain::GetInstance();

    httplib::Client cli(CLASSICUBE_NET_URL);

    httplib::Params params;
    params.emplace("name", Configuration::GenSettings.name);
    params.emplace("port", stringulate(Configuration::NetSettings.ListenPort));
    params.emplace("users", stringulate(nMain->roClients.size()));
    params.emplace("max", stringulate(Configuration::NetSettings.MaxPlayers));
    params.emplace("public", Configuration::NetSettings.Public ? "true" : "false");
    params.emplace("version", "7");
    params.emplace("salt", salt);
    params.emplace("software", "&e" + System::ServerName);

    auto res = cli.Post(CLASSICUBE_HEARTBEAT_PATH, params);
    if (!res) {
        Logger::LogAdd(MODULE_NAME, "Heartbeat failed.", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
    }
    else if (res->status != 200 || (res->body.find("http") == std::string::npos)) {
        Logger::LogAdd(MODULE_NAME, "Heartbeat failed.", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        json j = json::parse(res->body);
        Logger::LogAdd(MODULE_NAME, j["response"], LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
    } else {
        if (isFirstBeat) {
            Logger::LogAdd(MODULE_NAME, "Heartbeat sent.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
            serverUrl = res->body;
            Logger::LogAdd(MODULE_NAME, "Heartbeat URL: " + serverUrl, LogType::NORMAL, GLF);
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
    salt = CreateSalt();
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
