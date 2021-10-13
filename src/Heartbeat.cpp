//
// Created by unknown on 6/29/21.
//

#include "Heartbeat.h"


#include "System.h"
#include "TaskScheduler.h"
#include "Network.h"
#include "Player.h"
#include "network/httplib.h"
#include "digestpp/digestpp.hpp"
#include "Files.h"
#include "Utils.h"
#include "Logger.h"

const std::string MODULE_NAME = "Heartbeat";
Heartbeat* Heartbeat::Instance = nullptr;

void Heartbeat::Beat() {
    System* sMain = System::GetInstance();
    Network* nMain = Network::GetInstance();
    PlayerMain* pMain = PlayerMain::GetInstance();

    httplib::Client cli(CLASSICUBE_NET_URL);

    httplib::Params params;
    params.emplace("name", System::ServerName);
    params.emplace("port", stringulate(nMain->Port));
    params.emplace("users", stringulate(nMain->roClients.size()));
    params.emplace("max", stringulate(pMain->MaxPlayers));
    params.emplace("public", isPublic ? "true" : "false");
    params.emplace("version", "7");
    params.emplace("salt", salt);
    params.emplace("software", "&eD3PP");

    auto res = cli.Post(CLASSICUBE_HEARTBEAT_PATH, params);
    if (!res) {
        Logger::LogAdd(MODULE_NAME, "Heartbeat failed.", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
    }
    else if (res->status != 200 || (res->body.find("http") == std::string::npos)) {
        Logger::LogAdd(MODULE_NAME, "Heartbeat failed.", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        json j = json::parse(res->body);
        Logger::LogAdd(MODULE_NAME, j["response"], LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
    } else {
        Logger::LogAdd(MODULE_NAME, "Heartbeat sent.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
        serverUrl = res->body;
    }

    lastBeat = time(nullptr);
}

Heartbeat::Heartbeat() {
    SaveFile = false;
    LoadFile = true;
    salt = "";
    isPublic = false;
    lastBeat = time(nullptr);
    FileDateLast = 0;

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
    Load();
}

void Heartbeat::Load() {
    Files* fm = Files::GetInstance();
    std::string hbSettingsFile = fm->GetFile("Heartbeat");
    json j;
    std::ifstream iStream(hbSettingsFile);
    if (!iStream.is_open()) {
        Logger::LogAdd(MODULE_NAME, "Failed to load heartbeat settings, generating...", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        Save();
        return;
    }

    try {
        iStream >> j;
    } catch (int exception) {
        return;
    }
    iStream.close();

    isPublic = j["Public"];
    FileDateLast = Utils::FileModTime(hbSettingsFile);

    Logger::LogAdd(MODULE_NAME, "File Loaded", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void Heartbeat::Save() {
    Files* fm = Files::GetInstance();
    std::string hbSettingsFile = fm->GetFile("Heartbeat");
    json j;
    j["Public"] = isPublic;

    std::ofstream ofstream(hbSettingsFile);

    ofstream << std::setw(4) << j;
    ofstream.flush();
    ofstream.close();

    FileDateLast = Utils::FileModTime(hbSettingsFile);
    Logger::LogAdd(MODULE_NAME, "File Saved [" + hbSettingsFile + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void Heartbeat::MainFunc() {
    if (SaveFile) {
        Save();
        SaveFile = false;
    }

    Files* fm = Files::GetInstance();
    std::string hbSettingsFile = fm->GetFile("Heartbeat");

    time_t currentTime = Utils::FileModTime(hbSettingsFile);
    if (FileDateLast != currentTime || LoadFile) { // -- Check settings file for changes every 1 second.
        Load();
        LoadFile = false;
        FileDateLast = currentTime;
    }

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
