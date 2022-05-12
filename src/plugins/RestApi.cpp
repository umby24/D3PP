//
// Created by Wande on 4/19/2022.
//

#include "plugins/RestApi.h"

#include "network/httplib.h"
#include "common/Configuration.h"
#include "common/Logger.h"
#include "Rank.h"
#include "Block.h"
#include <json.hpp>
#include <network/Network.h>
#include "network/Network_Functions.h"
#include "Utils.h"

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

    m_restServer->Post("/settings/general", [](const httplib::Request& req, httplib::Response& res) {
        auto j = json::parse(req.body);
        Configuration::GenSettings.LoadFromJson(j);
        Configuration* cc = Configuration::GetInstance();
        cc->Save();

        json j2;
        Configuration::GenSettings.SaveToJson(j2);
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(stringulate(j2), "application/json");
    });

    m_restServer->Get("/settings/network",[](const httplib::Request& req, httplib::Response& res) {
        json j;
        Configuration::NetSettings.SaveToJson(j);
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(stringulate(j), "application/json");
    });
    m_restServer->Post("/settings/network",[](const httplib::Request& req, httplib::Response& res) {
        auto j = json::parse(req.body);
        Configuration::NetSettings.LoadFromJson(j);
        Configuration* cc = Configuration::GetInstance();
        cc->Save();

        json j2;
        Configuration::NetSettings.SaveToJson(j2);
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(stringulate(j2), "application/json");
    });

    m_restServer->Get("/settings/text",[](const httplib::Request& req, httplib::Response& res) {
        auto j = json::parse(req.body);
        Configuration::textSettings.LoadFromJson(j);
        Configuration* cc = Configuration::GetInstance();
        cc->Save();

        json j2;
        Configuration::textSettings.SaveToJson(j2);
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(stringulate(j2), "application/json");
    });

    m_restServer->Post("/settings/text",[](const httplib::Request& req, httplib::Response& res) {
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
    m_restServer->Get("/settings/blocks",[](const httplib::Request& req, httplib::Response& res) {
        Block* b = Block::GetInstance();
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(b->GetJson(), "application/json");
    });
    m_restServer->Get("/settings/customblocks",[](const httplib::Request& req, httplib::Response& res) {});
    m_restServer->Get("/settings/buildmodes",[](const httplib::Request& req, httplib::Response& res) {});

    m_restServer->Get("/world/maps",[](const httplib::Request& req, httplib::Response& res) {});
    m_restServer->Get("/network/players",[](const httplib::Request& req, httplib::Response& res) {
        Network* nMain = Network::GetInstance();

        json j;
        j["max"] = Configuration::NetSettings.MaxPlayers;
        j["current"] = nMain->roClients.size();
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(stringulate(j), "application/json");
    });

    m_restServer->Get("/plugins/",[](const httplib::Request& req, httplib::Response& res) {});
    m_restServer->Get("/playerdb",[](const httplib::Request& req, httplib::Response& res) {});
    m_restServer->Get("/system/log",[](const httplib::Request& req, httplib::Response& res) {
        json j;

        Logger* logMain = Logger::GetInstance();
        int numLines = 500;
        if (numLines > logMain->Messages.size()) {
            numLines = logMain->Messages.size();
        }
        int logSize = logMain->Messages.size();
        std::vector<json> items;

        for(int i = 0; i< numLines; i++) {

            LogMessage message = logMain->Messages.at(logSize-i-1);
            json obj = {
                    {"module", message.Module},
                    {"message", message.Message},
                    {"file", message.File},
                    {"line", message.Line},
                    {"function", message.Procedure},
                    {"type", message.Type},
                    {"time", message.Time}
            };
            items.push_back(obj);
        }
        j["log"] = items;
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(stringulate(j), "application/json");
    });
    m_restServer->Post("/system/message", [](const httplib::Request& req, httplib::Response& res){
       auto j = json::parse(req.body);
        if (j.is_object() && !j["message"].is_null()) {
            NetworkFunctions::SystemMessageNetworkSend2All(-1, "&c[&fCONSOLE&c]:&f " + j["message"].get<std::string>());
        }
    });
    std::thread apiThread ([this]{ RunHttpServer(); });
    std::swap(m_serverThread, apiThread);
    Logger::LogAdd(MODULE_NAME, "API Running on port 8080", LogType::NORMAL, GLF);
}

void RestApi::RunHttpServer() {
    m_restServer->listen("localhost", 8080);
}
