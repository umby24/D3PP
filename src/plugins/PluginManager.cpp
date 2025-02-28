//
// Created by Wande on 2/10/2022.
//

#include "plugins/PluginManager.h"
#include "plugins/LuaPlugin.h"

#include <vector>
#include <string>
#include <filesystem>
#include <common/Files.h>
#include <Utils.h>
#include <common/Logger.h>

using namespace D3PP::plugins;

PluginManager* PluginManager::Instance = nullptr;

std::vector<std::string> GetDirectories(const std::string &dir) {
    std::vector<std::string> result{};

    if (std::filesystem::is_directory(dir)) {
        for (const auto &entry : std::filesystem::directory_iterator(dir)) {
            std::string fileName = entry.path().filename().string();

            if (fileName.length() < 3) // -- exclude . and ..
                continue;

            if (entry.is_directory()) {
                result.push_back(entry.path().string());
                continue;
            }
        }
    }

    return result;
}

void PluginManager::RefreshPluginList() {
    std::string pluginBaseDirectory = Files::GetFolder("Plugins");
    std::vector<std::string> pluginDirectories = GetDirectories(pluginBaseDirectory);

    for (auto &plugin : m_plugins) { // -- Mark unfound plugins for removal.
        if (std::find(pluginDirectories.begin(), pluginDirectories.end(), plugin->GetFolderName()) == pluginDirectories.end()) {
            plugin->m_status = "NotFound";
        }
    }

    // -- Create everything new.
    for(auto &folder : pluginDirectories) {
        bool alreadyHave = false;

        for (auto const &plugin : m_plugins) {
            if (plugin->GetFolderName() == folder) {
                alreadyHave = true;
                break;
            }
        }

        if (!alreadyHave) {
            m_plugins.push_back(std::make_shared<LuaPlugin>(folder));
        }
    }
}

void PluginManager::LoadPlugins() {
    Logger::LogAdd("PluginManager", "Loading Plugins", NORMAL, GLF);
    RefreshPluginList();
    // -- TODO: Maybe? Make plugins only load if they are in a plugins settings file.
    int numLoadedPlugins = 0;

    for(auto & plugin : m_plugins) {
        if (!plugin->IsLoaded()) {
            plugin->Load();
        }
        else {
            plugin->Unload();
            plugin->Load();
        }
    }

    Logger::LogAdd("PluginManager", "Loaded " + stringulate(m_plugins.size()) + " plugins.", NORMAL, GLF);
}

PluginManager *PluginManager::GetInstance() {
    if (Instance == nullptr) {
        Instance = new PluginManager();
    }

    return Instance;
}

void PluginManager::TriggerCommand(const std::string &function, int clientId, const std::string &parsedCmd,
                                   const std::string &text0, const std::string &text1, const std::string &op1,
                                   const std::string &op2, const std::string &op3, const std::string &op4,
                                   const std::string &op5) const {
    for(auto & plugin : m_plugins) {
        if (plugin->IsLoaded()) {
            plugin->TriggerCommand(function, clientId, parsedCmd, text0, text1, op1, op2, op3, op4, op5);
        }
    }

}

void PluginManager::TriggerMapFill(int mapId, int sizeX, int sizeY, int sizeZ, const std::string &function,
                                   const std::string &args) const {
    for(auto & plugin : m_plugins) {
        if (plugin->IsLoaded()) {
            plugin->TriggerMapFill(mapId, sizeX, sizeY, sizeZ, function, args);
        }
    }
}

void PluginManager::TriggerPhysics(int mapId, unsigned short X, unsigned short Y, unsigned short Z,
                                   const std::string &function) const {
    for(auto & plugin : m_plugins) {
        if (plugin->IsLoaded()) {
            plugin->TriggerPhysics(mapId, X, Y, Z, function);
        }
    }
}

void PluginManager::TriggerBuildMode(const std::string &function, int clientId, int mapId, unsigned short X,
                                     unsigned short Y, unsigned short Z, unsigned char mode, unsigned char block) const {
    for(auto & plugin : m_plugins) {
        if (plugin->IsLoaded()) {
            plugin->TriggerBuildMode(function, clientId, mapId, X, Y, Z, mode, block);
        }
    }
}

void PluginManager::TriggerBlockCreate(const std::string& function, int mapId, unsigned short X, unsigned short Y, unsigned short Z) const {
    for (auto& plugin : m_plugins) {
        if (plugin->IsLoaded()) {
            plugin->TriggerBlockCreate(function, mapId, X, Y, Z);
        }
    }
}

void PluginManager::TriggerBlockDelete(const std::string& function, int mapId, unsigned short X, unsigned short Y, unsigned short Z) const {
    for (auto& plugin : m_plugins) {
        if (plugin->IsLoaded()) {
            plugin->TriggerBlockDelete(function, mapId, X, Y, Z);
        }
    }
}
