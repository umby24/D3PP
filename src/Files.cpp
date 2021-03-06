#include "Files.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include "Utils.h"
#include "Logger.h"

Files* Files::singleton_ = nullptr;;
const std::string MODULE_NAME = "Files";

Files *Files::GetInstance() {
    if (singleton_ == nullptr) {
        singleton_ = new Files();
    }
    return singleton_;
}

Files::Files()
{
    //ctor
    Load();
}

Files::~Files()
{
    folders.clear();
    files.clear();
    //dtor
}

std::string Files::GetFile(std::string name) {
    if (files.find(name) == files.end()) {
        Logger::LogAdd(MODULE_NAME, "Path to file [" + name + "] not defined", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
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
        Logger::LogAdd(MODULE_NAME, "Path to folder [" + name + "] not defined", LogType::WARNING, __FILE__, __LINE__, __FUNCTION__);
        return "";
    }

    return folders[name];
}

void Files::LoadDefault() {
    Logger::LogAdd(MODULE_NAME, "Files.json not found, generating default.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    folders = { {"Main", ""}, {"Data", "Data/"}, {"Heartbeat", "Heartbeat/"}, {"Logs", "Logs/"}, {"HTML", "HTML/"} };
    files = {{"Answer", "[Main][Data]Answer.txt"}, {"Block", "[Main][Data]Block.txt"}, {"Log", "[Main][Logs]Log_[i].txt"},
    {"Map_List", "[Main][Data]MapList.txt"}, {"Map_HTML", "[Main][HTML]Maps.html"}};
}

void Files::Load() {
    if (Utils::FileSize("files.json") == -1) {
        LoadDefault();
        Save();
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
