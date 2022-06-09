//
// Created by Wande on 4/19/2022.
//

#include "plugins/RestApi.h"

#include "world/Map.h"
#include "network/httplib.h"
#include "common/Configuration.h"
#include "common/Logger.h"
#include "Rank.h"
#include "Block.h"
#include <json.hpp>
#include <network/Network.h>
#include "network/Network_Functions.h"
#include "network/NetworkClient.h"

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
    m_restServer->set_mount_point("/", "./www");
    m_restServer->Options("/settings/general", [](const httplib::Request& req, httplib::Response& res){
        res.set_header("Allow", "GET,POST");
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "content-type");
    });
    m_restServer->Get("/settings/general", [](const httplib::Request& req, httplib::Response& res) {
        json j;
        Configuration::GenSettings.SaveToJson(j);
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(stringulate(j), "application/json");
    });

    m_restServer->Post("/settings/general", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        auto j = json::parse(req.body);
        Configuration::GenSettings.LoadFromJson(j);
        Configuration* cc = Configuration::GetInstance();
        cc->Save();

        json j2;
        Configuration::GenSettings.SaveToJson(j2);

        res.set_content(stringulate(j2), "application/json");
    });
    m_restServer->Options("/settings/network", [](const httplib::Request& req, httplib::Response& res){
        res.set_header("Allow", "GET,POST");
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "content-type");
    });
    m_restServer->Get("/settings/network",[](const httplib::Request& req, httplib::Response& res) {
        json j;
        Configuration::NetSettings.SaveToJson(j);
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(stringulate(j), "application/json");
    });
    m_restServer->Post("/settings/network",[](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        auto j = json::parse(req.body);
        Configuration::NetSettings.LoadFromJson(j);
        Configuration* cc = Configuration::GetInstance();
        cc->Save();

        json j2;
        Configuration::NetSettings.SaveToJson(j2);

        res.set_content(stringulate(j2), "application/json");
    });

    m_restServer->Options("/settings/text", [](const httplib::Request& req, httplib::Response& res){
        res.set_header("Allow", "GET,POST");
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "content-type");
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
        auto j = json::parse(req.body);
        Configuration::textSettings.LoadFromJson(j);
        Configuration* cc = Configuration::GetInstance();
        cc->Save();

        json j2;
        Configuration::textSettings.SaveToJson(j2);
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(stringulate(j2), "application/json");
    });
    m_restServer->Options("/settings/ranks", [](const httplib::Request& req, httplib::Response& res){
        res.set_header("Allow", "GET,POST");
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "content-type");
    });
    m_restServer->Get("/settings/ranks",[](const httplib::Request& req, httplib::Response& res) {
        Rank* rm = Rank::GetInstance();
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(rm->GetJson(), "application/json");
    });
    m_restServer->Post("/settings/ranks", [](const httplib::Request &req, httplib::Response &res) {
        auto j = json::parse(req.body);

        Rank *rm = Rank::GetInstance();
        rm->SetJson(j);
        rm->Save();

        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(rm->GetJson(), "application/json");
    });
    m_restServer->Delete(R"(/settings/ranks/(\d+))", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        int rankId = std::stoi(req.matches[1].str().c_str());

        Rank *rm = Rank::GetInstance();
        rm->Delete(rankId, true);
        rm->Save();

        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(rm->GetJson(), "application/json");
    });

    m_restServer->Options("/settings/blocks", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Allow", "GET,POST");
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "content-type");
    });
    m_restServer->Get("/settings/blocks", [](const httplib::Request &req, httplib::Response &res) {
        Block *b = Block::GetInstance();
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(b->GetJson(), "application/json");
    });
    m_restServer->Post("/settings/blocks", [](const httplib::Request &req, httplib::Response &res) {
        auto j = json::parse(req.body);
        Block *b = Block::GetInstance();
        b->SetJson(j);
        b->Save();

        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(b->GetJson(), "application/json");
    });
    m_restServer->Delete(R"(/settings/blocks/(\d+))", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        int blockId = std::stoi(req.matches[1].str().c_str());

        auto j = json::parse(req.body);
        Block *b = Block::GetInstance();

        b->DeleteBlock(blockId);
        b->Save();


        res.set_content(b->GetJson(), "application/json");
    });
    m_restServer->Get("/settings/customblocks", [](const httplib::Request &req, httplib::Response &res) {});
    m_restServer->Get("/settings/buildmodes", [](const httplib::Request &req, httplib::Response &res) {});

    m_restServer->Options("/world/maps", [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Allow", "GET,POST");
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "content-type");
    });
    m_restServer->Get("/world/maps",[](const httplib::Request& req, httplib::Response& res) {
        D3PP::world::MapMain* mm = D3PP::world::MapMain::GetInstance();
        json j;
        std::vector<json> items;
        for (auto const &m : mm->_maps) {
            json ji;
            ji["id"] = m.first;
            ji["name"] = m.second->Name();
            ji["loaded"] = m.second->loaded;
            ji["numClients"] = m.second->Clients;
            items.push_back(ji);
        }
        j["maps"] = items;
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_content(stringulate(j), "application/json");
    });
    m_restServer->Post("/world/maps",[](const httplib::Request& req, httplib::Response& res) {
        auto j = json::parse(req.body);
        json resp;
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");

        if (j.contains("x") == false || j.contains("y") == false || j.contains("z") == false || j.contains("name") == false) {
            resp["error"] = "Invalid request";
            res.set_content(stringulate(resp), "application/json");
            res.status = 400;
            return;
        }

        if (!j["x"].is_number() || !j["y"].is_number() || !j["z"].is_number()) {
            resp["error"] = "Invalid request, size must be of integer type.";
            res.set_content(stringulate(resp), "application/json");
            res.status = 400;
            return;
        }

        if (j["x"] > 512 || j["y"] > 512 || j["z"] > 512 || j["x"] <= 0 || j["y"] <= 0 || j["z"] <= 0) {
            resp["error"] = "Invalid request, map size must be 1-512 blocks on each axis.";
            res.set_content(stringulate(resp), "application/json");
            res.status = 400;
            return;
        }

        D3PP::world::MapMain* mm = D3PP::world::MapMain::GetInstance();
        int resultId = mm->Add(-1, j["x"], j["y"], j["z"], j["name"]);
        resp["id"] = resultId;
        res.set_content(stringulate(resp), "application/json");
    });

    m_restServer->Get("/network/players",[](const httplib::Request& req, httplib::Response& res) {
        Network* nMain = Network::GetInstance();

        json j;
        j["max"] = Configuration::NetSettings.MaxPlayers;
        j["current"] = nMain->roClients.size();
        auto jPlayers = json::array();

        for (auto const &nc : nMain->roClients) {
            jPlayers.push_back(nc->GetLoginName());
        }

        j["list"] = jPlayers;
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
    m_restServer->Options("/system/message", [](const httplib::Request& req, httplib::Response& res){
        res.set_header("Allow", "POST");
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
        res.set_header("Access-Control-Allow-Headers", "content-type");
    });

    m_restServer->Post("/system/message", [](const httplib::Request& req, httplib::Response& res){
       auto j = json::parse(req.body);
        if (j.is_object() && !j["message"].is_null()) {
            NetworkFunctions::SystemMessageNetworkSend2All(-1, "&c[&fCONSOLE&c]:&f " + j["message"].get<std::string>());
            Logger::LogAdd("RestAPI", "[CONSOLE]: " + j["message"].get<std::string>(), CHAT, GLF);
        }
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
    });

    m_restServer->set_error_handler([](const auto& req, auto& res) {
        auto fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
        char buf[BUFSIZ];
        snprintf(buf, sizeof(buf), fmt, res.status);
        res.set_content(buf, "text/html");
    });
    m_restServer->set_exception_handler([](const auto& req, auto& res, std::exception &e) {
        res.status = 500;
        auto fmt = "<h1>Error 500</h1><p>%s</p>";
        char buf[BUFSIZ];
        snprintf(buf, sizeof(buf), fmt, e.what());
        res.set_content(buf, "text/html");
    });
    std::thread apiThread ([this]{ RunHttpServer(); });
    std::swap(m_serverThread, apiThread);
    Logger::LogAdd(MODULE_NAME, "API Running on port 8080", LogType::NORMAL, GLF);
}

void RestApi::RunHttpServer() {
    m_restServer->listen("localhost", 8080);
}
