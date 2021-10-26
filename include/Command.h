//
// Created by Wande on 6/4/2021.
//

#ifndef D3PP_COMMAND_H
#define D3PP_COMMAND_H

#include <string>
#include <vector>
#include <chrono>
#include <memory>

#include "common/TaskScheduler.h"

class Network;
class NetworkClient;

const int COMMAND_OPERATORS_MAX = 5;
const std::string COMMAND_FILENAME = "Command";

struct CommandGroup {
    std::string Name;
    short RankShow;
};

class Command {
    public:
        std::string Id;
        std::string Name;
        std::string Group;
        std::string Description;
        short Rank;
        short RankShow;
        std::string Plugin;
        std::function<void()> Function;
        bool Internal;
        bool Hidden;
};

class CommandMain : TaskItem {
    public:

        bool SaveFile;
        time_t FileDateLast;
        int CommandClientId;
        std::string ParsedCommand;
        std::vector<std::string> ParsedOperator[COMMAND_OPERATORS_MAX];
        std::string ParsedText0;
        std::string ParsedText1;
        std::string ParsedText2;
        std::vector<CommandGroup> CommandGroups;
        std::vector<Command> Commands;
        
        static CommandMain* GetInstance();
        static CommandMain* Instance;

        CommandMain();
        void Init();
        void Load();
        void MainFunc();
        void CommandDo(const std::shared_ptr<NetworkClient>& client, const std::string& input);
        // -- Administrative
        void CommandKick();
        void CommandBan();
        void CommandUnban();
        void CommandStop();
        void CommandUnStop();
        void CommandMute();
        void CommandUnmute();
        void CommandServerInfo();
        void CommandLogLast();
        void CommandGetRank();

        // -- General
        void CommandCommands();
        void CommandHelp();
        void CommandPlayers();
        void CommandPlayerInfo();
        void CommandGlobal();
        void CommandPing();
        void CommandChangeMap();

        void CommandListMaps();
        void CommandChangeRank();
        void CommandTeleport();
        void CommandBring();

        // -- Map Modifiers
        void CommandSaveMap();
        void CommandLoadMap();
        void CommandResizeMap();
        void CommandRenameMap();
        void CommandDeleteMap();
        void CommandAddMap();
        void CommandMapFill();
        void CommandMapBlockCount();
        void CommandMapRankBuildSet();
        void CommandMapRankJoinSet();
        void CommandMapRankShowSet();
        void CommandStopPhysics();
        void CommandStartPhysics();
        void CommandSetSpawn();
        void CommandSetKilLSpawn();
        void CommandTeleporters();
        void CommandDeleteTeleporter();
        void CommandMapInfo();

        // -- Build tools
        void CommandSetMaterial();
        void CommandMaterials();
        void CommandPlace();
        void CommandUndoTime();
        void CommandUndoPlayer();
        void CommandUndo();
        void CommandUserMaps();
        // -- Commands not ported: Player Attribute Get, map directory rename, map blocks count, set/delete/bring/tp to location, time.

    private:
};


#endif