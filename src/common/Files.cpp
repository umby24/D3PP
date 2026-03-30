#include "common/Files.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include "Utils.h"
#include "common/Logger.h"
#include <json.hpp>

using json = nlohmann::json;

const std::string MODULE_NAME = "Files";
std::map<std::string, std::string> Files::files;
std::map<std::string, std::string> Files::folders;

std::string Files::GetFile(std::string name) {
    if (files.find(name) == files.end()) {
        return "";
    }

    std::string result = files[name];

    for (auto const& x : folders) {
        Utils::replaceAll(result, "[" + x.first + "]", x.second);
    }

    return result;
}

std::string Files::GetFolder(std::string name) {
    if (folders.find(name) == folders.end()) {
        Logger::LogAdd(MODULE_NAME, "Path to folder [" + name + "] not defined", WARNING, __FILE__, __LINE__, __FUNCTION__);
        return "";
    }

    return folders[name];
}

void Files::LoadDefault() {
    std::cout << "Files.json not found, generating default.\n";

    folders = {
            {"Main", ""},
            {"Data", "Data/"},
            {"Heartbeat", "Heartbeat/"},
            {"Logs", "Logs/"},
            {"HTML", "HTML/"},
            {"Temp", "Temp/"},
            {"Maps", "HTML/"},
            {"CWMaps", "CWMaps/"},
            {"Usermaps", "Usermaps/"},
            {"Plugins", "Plugins/"},
    };
    files = {
            {"Block", "[Main][Data]Block.json"},
            {"Build_Mode", "[Main][Data]Buildmode.txt"},
            {"Command", "[Main][Data]Commands.json"},
            {"Log", "[Main][Logs]Log_[i].txt"},
            {"Heartbeat", "[Main][Data]heartbeat.json"},
            {"Rank", "[Main][Data]Ranks.json"},
            {"Map_List", "[Main][Data]MapList.txt"},
            {"Map_Settings", "[Main][Data]Map_Settings.txt"},
            {"System", "[Main][Data]System.json"},
            {"Network", "[Main][Data]Network.json"},
            {"Player", "[Main][Data]Player.json"},
            {"Map_HTML", "[Main][HTML]Maps.html"},
            {"Mem_HTML", "[Main][HTML]Mem.html"},
            {"Network_HTML", "[Main][HTML]Network.html"},
            {"Playerlist", "[Main][Data]playerlist.sqlite3"},
            {"Watchdog_HTML", "[Main][HTML]Watchdog.html"},
            {"configuration", "[Main][Data]Config.json"},
            {"BlockDefs", "[Main][Data]BlockDefs.json"},
    };
}

void Files::Load() {
    if (Utils::FileSize("files.json") == -1) {
        LoadDefault();
        Save();
        Load();
        return;
    }

    json j;
    std::ifstream iStream("files.json");

    if (iStream.is_open()) {
        iStream >> j;
        iStream.close();
    }

    if (j["folders"] != nullptr) {
        for (auto& element : j["folders"].items()) {
            folders[element.key()] = element.value();
            Utils::DirectoryExists(element.value(), true);
        }
    }

    if (j["files"] != nullptr) {
        for (auto& element : j["files"].items()) {
            files[element.key()] = element.value();
        }
    }
}

void Files::Save() {
    json j;

    j["folders"] = nullptr;
    j["files"] = nullptr;

    for (auto const& f : folders) {
        j["folders"][f.first] = f.second;
    }

    for (auto const& f : files) {
        j["files"][f.first] = f.second;
    }

    std::ofstream oStream;
    oStream.open("files.json", std::ios::out | std::ios::trunc);
    oStream << std::setw(4) << j << std::endl;
    oStream.flush();
    oStream.close();
}
