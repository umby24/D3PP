//
// Created by Wande on 4/19/2022.
//

#include "plugins/RestApi.h"

#include "network/httplib.h"
#include "common/Configuration.h"
#include "Rank.h"
#include <json.hpp>
#include <common/Logger.h>
#include <Utils.h>

const std::string MODULE_NAME = "RestAPI";
using json = nlohmann::json;

RestApi::RestApi() {
    this->Setup = [this] { Init(); };
    m_restServer = nullptr;
    TaskScheduler::RegisterTask(MODULE_NAME, *this);
}

void RestApi::Init() {
    m_restServer = std::make_shared<httplib::Server>();

    m_restServer->Get("/settings/general", [](const httplib::Request& req, httplib::Response& res) {
        json j;
        Configuration::GenSettings.SaveToJson(j);
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(stringulate(j), "application/json");
    });
    m_restServer->Get("/settings/network",[](const httplib::Request& req, httplib::Response& res) {
        json j;
        Configuration::NetSettings.SaveToJson(j);
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(stringulate(j), "application/json");
    });
    m_restServer->Get("/settings/text",[](const httplib::Request& req, httplib::Response& res) {
        json j;
        Configuration::textSettings.SaveToJson(j);
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(stringulate(j), "application/json");
    });
    m_restServer->Get("/settings/ranks",[](const httplib::Request& req, httplib::Response& res) {
        Rank* rm = Rank::GetInstance();
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(rm->GetJson(), "application/json");
    });
    m_restServer->Get("/settings/blocks",[](const httplib::Request& req, httplib::Response& res) {});
    m_restServer->Get("/settings/customblocks",[](const httplib::Request& req, httplib::Response& res) {});
    m_restServer->Get("/settings/buildmodes",[](const httplib::Request& req, httplib::Response& res) {});

    m_restServer->Get("/world/maps",[](const httplib::Request& req, httplib::Response& res) {});
    m_restServer->Get("/network/players",[](const httplib::Request& req, httplib::Response& res) {});
    m_restServer->Get("/plugins/",[](const httplib::Request& req, httplib::Response& res) {});
    m_restServer->Get("/playerdb",[](const httplib::Request& req, httplib::Response& res) {});
    m_restServer->Get("/system/log",[](const httplib::Request& req, httplib::Response& res) {});

    std::thread apiThread ([this]{ RunHttpServer(); });
    std::swap(m_serverThread, apiThread);
    Logger::LogAdd(MODULE_NAME, "API Running on port 8080", LogType::NORMAL, GLF);
}

void RestApi::RunHttpServer() {
    m_restServer->listen("localhost", 8080);
}
