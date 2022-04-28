//
// Created by Wande on 2/4/2022.
//

#ifndef D3PP_PLUGINMANAGER_H
#define D3PP_PLUGINMANAGER_H

#include <vector>
#include <memory>
#include <string>
class LuaPlugin;

namespace D3PP::plugins {
    class PluginManager {
    public:
        static PluginManager* GetInstance();
        void UnloadPlugin();
        void ReloadPlugin();
        void LoadPlugin();
        void RemoveHooks();
        bool IsPluginLoaded();

        void LoadPlugins();
        // -- ExecuteCommand..
        // -- ForEach plugin

        void TriggerCommand(const std::string& function, int clientId, const std::string& parsedCmd, const std::string &text0, const std::string &text1, const std::string& op1, const std::string &op2, const std::string &op3, const std::string &op4, const std::string &op5);
        void TriggerMapFill(int mapId, int sizeX, int sizeY, int sizeZ, const std::string& function, const std::string& args);
        void TriggerPhysics(int mapId, unsigned short X, unsigned short Y, unsigned short Z, const std::string& function);
        void TriggerBuildMode(const std::string &function, int clientId, int mapId, unsigned short X, unsigned short Y, unsigned short Z, unsigned char mode, unsigned char block);
    private:
        static PluginManager* Instance;
        void RefreshPluginList();
        std::vector<std::shared_ptr<LuaPlugin>> m_plugins;
    };
}
#endif //D3PP_PLUGINMANAGER_H
