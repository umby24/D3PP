#include "Command.h"

#include "common/PreferenceLoader.h"
#include "network/NetworkClient.h"
#include "network/Network.h"
#include "network/Server.h"
#include "world/Player.h"
#include "common/Player_List.h"
#include "world/Entity.h"
#include "Block.h"
#include "world/Map.h"
#include "world/MapMain.h"
#include "Rank.h"
#include "common/Logger.h"
#include "common/Files.h"
#include "System.h"
#include "Utils.h"
#include "plugins/PluginManager.h"

const std::string MODULE_NAME = "Command";
CommandMain* CommandMain::Instance = nullptr;

using namespace D3PP::world;
using namespace D3PP::Common;

CommandMain::CommandMain() : ParsedOperator{} {
    this->Setup = [this] { Load(); };
    this->Main = [this] { MainFunc(); };
    this->Interval = std::chrono::seconds(1);
    this->LastRun = std::chrono::system_clock::now();

    SaveFile = false;
    FileDateLast = 0;
    CommandClientId = -1;
    ParsedOperator.emplace_back("");
    ParsedOperator.emplace_back("");
    ParsedOperator.emplace_back("");
    ParsedOperator.emplace_back("");
    ParsedOperator.emplace_back("");
    TaskScheduler::RegisterTask("Commands", *this);
}

void CommandMain::Init() {
    Command PluginReloadCommand;
    PluginReloadCommand.Function = [this] { D3PP::plugins::PluginManager::GetInstance()->LoadPlugins(); };
    PluginReloadCommand.Id = "preload";
    PluginReloadCommand.Name = "preload";
    PluginReloadCommand.Internal = true;
    PluginReloadCommand.Hidden = false;
    PluginReloadCommand.Rank = 260;
    PluginReloadCommand.RankShow = 260;
    PluginReloadCommand.CanConsole = true;
    PluginReloadCommand.Description = "Reloads all Lua Plugins";
    PluginReloadCommand.Group = "Plugins";
    Commands.push_back(PluginReloadCommand);

    Command kickCommand;
    kickCommand.Id = "Kick";
    kickCommand.Name = "kick";
    kickCommand.Internal = true;
    kickCommand.Hidden = false;
    kickCommand.Rank = 150;
    kickCommand.RankShow = 150;
    kickCommand.CanConsole = true;
    kickCommand.Function = [this] { CommandKick(); };
    kickCommand.Description = "kick [player] - Kicks a player from the server";
    kickCommand.Group = "Admin";
    Commands.push_back(kickCommand);

    Command banCommand;
    banCommand.Id = "Ban";
    banCommand.Name = "ban";
    banCommand.Internal = true;
    banCommand.Hidden = false;
    banCommand.Rank = 150;
    banCommand.RankShow = 150;
    banCommand.CanConsole = true;
    banCommand.Function = [this] { CommandBan(); };
    banCommand.Description = "ban [player] - Bans from the server";
    banCommand.Group = "Admin";
    Commands.push_back(banCommand);

    Command unbanCommand;
    unbanCommand.Id = "Un-Ban";
    unbanCommand.Name = "unban";
    unbanCommand.Internal = true;
    unbanCommand.Hidden = false;
    unbanCommand.Rank = 150;
    unbanCommand.CanConsole = true;
    unbanCommand.RankShow = 150;
    unbanCommand.Function = [this] { CommandUnban(); };
    unbanCommand.Description = "unban [player] - Unbans from the server";
    unbanCommand.Group = "Admin";
    Commands.push_back(unbanCommand);

    Command stopcmd;
    stopcmd.Id = "Stop-Player";
    stopcmd.Name = "stop";
    stopcmd.Internal = true;
    stopcmd.Hidden = false;
    stopcmd.Rank = 150;
    stopcmd.RankShow = 150;
    stopcmd.CanConsole = true;
    stopcmd.Function = [this] { CommandStop(); };
    stopcmd.Description = "stop [player] - Stops a player from building";
    stopcmd.Group = "Admin";
    Commands.push_back(stopcmd);

    Command unstopCmd;
    unstopCmd.Id = "Un-Stop";
    unstopCmd.Name = "unstop";
    unstopCmd.Internal = true;
    unstopCmd.Hidden = false;
    unstopCmd.Rank = 150;
    unstopCmd.RankShow = 150;
    unstopCmd.CanConsole = true;
    unstopCmd.Function = [this] { CommandUnStop(); };
    unstopCmd.Description = "unstop [player] - Unstops a player, allowing them to build again.";
    unstopCmd.Group = "Admin";
    Commands.push_back(unstopCmd);

    Command muteCmd;
    muteCmd.Id = "Mute-Player";
    muteCmd.Name = "mute";
    muteCmd.Internal = true;
    muteCmd.Hidden = false;
    muteCmd.Rank = 150;
    muteCmd.RankShow = 150;
    muteCmd.CanConsole = true;
    muteCmd.Function = [this] { CommandMute(); };
    muteCmd.Description = "mute [player] [minutes] - Prevent a player from chatting for a time.";
    muteCmd.Group = "Admin";
    Commands.push_back(muteCmd);

    Command unMuteCmd;
    unMuteCmd.Id = "Unmute-Player";
    unMuteCmd.Name = "unmute";
    unMuteCmd.Internal = true;
    unMuteCmd.Hidden = false;
    unMuteCmd.Rank = 150;
    unMuteCmd.RankShow = 150;
    unMuteCmd.CanConsole = true;
    unMuteCmd.Function = [this] { CommandUnmute(); };
    muteCmd.Description = "unmute [player] - Allow a player to chat again.";
    muteCmd.Group = "Admin";
    Commands.push_back(unMuteCmd);

    Command listCommands;
    listCommands.Id = "List-Commands";
    listCommands.Name = "commands";
    listCommands.Internal = true;
    listCommands.Hidden = false;
    listCommands.Rank = 0;
    listCommands.RankShow = 0;
    listCommands.CanConsole = true;
    listCommands.Function = [this] { CommandCommands(); };
    listCommands.Description = "commands [group(opt)] - Displays all commands, optionally within a specific group.";
    listCommands.Group = "Common";
    Commands.push_back(listCommands);

    Command helpCommand;
    helpCommand.Id = "Command-Help";
    helpCommand.Name = "cmdhelp";
    helpCommand.Internal = true;
    helpCommand.Hidden = false;
    helpCommand.Rank = 0;
    helpCommand.RankShow = 0;
    helpCommand.CanConsole = true;
    helpCommand.Function = [this] { CommandHelp(); };
    helpCommand.Description = "help [cmd] - Displays the help text of a command.";
    helpCommand.Group = "Common";
    Commands.push_back(helpCommand);

    Command listPlayers;
    listPlayers.Id = "List-Players";
    listPlayers.Name = "players";
    listPlayers.Internal = true;
    listPlayers.Hidden = false;
    listPlayers.Rank = 0;
    listPlayers.RankShow = 0;
    listPlayers.CanConsole = true;
    listPlayers.Function = [this] { CommandPlayers(); };
    listPlayers.Description = "players - Lists all players on the server and in which map they are in.";
    listPlayers.Group = "Common";
    Commands.push_back(listPlayers);

    Command pInfoCmd;
    pInfoCmd.Id = "Player-Info";
    pInfoCmd.Name = "pinfo";
    pInfoCmd.Internal = true;
    pInfoCmd.Hidden = false;
    pInfoCmd.Rank = 0;
    pInfoCmd.RankShow = 0;
    pInfoCmd.CanConsole = true;
    pInfoCmd.Function = [this] { CommandPlayerInfo(); };
    pInfoCmd.Description = "pinfo [player] - Displays the playerDB stats of this user.";
    pInfoCmd.Group = "Admin";
    Commands.push_back(pInfoCmd);

    Command pingCommand;
    pingCommand.Id = "Ping";
    pingCommand.Name = "ping";
    pingCommand.Internal = true;
    pingCommand.Hidden = false;
    pingCommand.Rank = 0;
    pingCommand.RankShow = 0;
    pingCommand.CanConsole = true;
    pingCommand.Function = [this] { CommandPing(); };
    pingCommand.Description = "Displays the network delay between yourself and the server.";
    pingCommand.Group = "Common";
    Commands.push_back(pingCommand);

    Command globalCommand;
    globalCommand.Id = "Global";
    globalCommand.Name = "global";
    globalCommand.Internal = true;
    globalCommand.Hidden = false;
    globalCommand.Rank = 0;
    globalCommand.RankShow = 0;
    globalCommand.CanConsole = true;
    globalCommand.Function = [this] { CommandGlobal(); };
    globalCommand.Description = "global - Toggles global chat by default on or off.";
    globalCommand.Group = "Chat";
    Commands.push_back(globalCommand);

    Command changeMapCommand;
    changeMapCommand.Id = "Map";
    changeMapCommand.Name = "map";
    changeMapCommand.Internal = true;
    changeMapCommand.Hidden = false;
    changeMapCommand.Rank = 0;
    changeMapCommand.RankShow = 0;
    changeMapCommand.CanConsole = false;
    changeMapCommand.Function = [this] { CommandChangeMap(); };
    changeMapCommand.Description = "map [name] - Change to a new map.";
    changeMapCommand.Group = "Map";
    Commands.push_back(changeMapCommand);

    Command changeRankCommand;
    changeRankCommand.Id = "Set-Rank";
    changeRankCommand.Name = "setrank";
    changeRankCommand.Internal = true;
    changeRankCommand.Hidden = false;
    changeRankCommand.Rank = 150;
    changeRankCommand.RankShow = 150;
    changeRankCommand.CanConsole = true;
    changeRankCommand.Function = [this] { CommandChangeRank(); };
    changeRankCommand.Description = "setrank [name] [-1 - 65535]- Change a player to a new rank";
    changeRankCommand.Group = "Admin";
    Commands.push_back(changeRankCommand);

    Command mapSaveCommand;
    mapSaveCommand.Id = "Map-Save";
    mapSaveCommand.Name = "mapsave";
    mapSaveCommand.Internal = true;
    mapSaveCommand.Hidden = false;
    mapSaveCommand.Rank = 150;
    mapSaveCommand.RankShow = 150;
    mapSaveCommand.CanConsole = false;
    mapSaveCommand.Function = [this] { CommandSaveMap(); };
    mapSaveCommand.Description = "Saves the map you are on.";
    mapSaveCommand.Group = "Map";
    Commands.push_back(mapSaveCommand);

    Command getRankCommand;
    getRankCommand.Id = "Get-Rank";
    getRankCommand.Name = "getrank";
    getRankCommand.Internal = true;
    getRankCommand.Hidden = false;
    getRankCommand.Rank = 0;
    getRankCommand.RankShow = 0;
    getRankCommand.CanConsole = true;
    getRankCommand.Function = [this] { CommandGetRank(); };
    getRankCommand.Description = "getrank [player] - Gets the rank of a player.";
    getRankCommand.Group = "Common";
    Commands.push_back(getRankCommand);

    Command setMaterialCommand;
    setMaterialCommand.Id = "Material";
    setMaterialCommand.Name = "material";
    setMaterialCommand.Internal = true;
    setMaterialCommand.Hidden = false;
    setMaterialCommand.Rank = 0;
    setMaterialCommand.RankShow = 0;
    setMaterialCommand.CanConsole = false;
    setMaterialCommand.Function = [this] { CommandSetMaterial(); };
    setMaterialCommand.Description = "material [block name] - Binds stone to instead build this material. /cancel to stop.";
    setMaterialCommand.Group = "Building";
    Commands.push_back(setMaterialCommand);

    Command materialList;
    materialList.Id = "List-Materials";
    materialList.Name = "materials";
    materialList.Internal = true;
    materialList.Hidden = false;
    materialList.Rank = 0;
    materialList.RankShow = 0;
    materialList.CanConsole = true;
    materialList.Function = [this] { CommandMaterials(); };
    materialList.Description = "Displays a list of all custom blocks on the server.";
    materialList.Group = "Building";
    Commands.push_back(materialList);

    Command undoPLayer;
    undoPLayer.Id = "Undo-Player";
    undoPLayer.Name = "undoplayer";
    undoPLayer.Internal = true;
    undoPLayer.Hidden = false;
    undoPLayer.Rank = 150;
    undoPLayer.RankShow = 150;
    undoPLayer.CanConsole = true;
    undoPLayer.Function = [this] { CommandUndoPlayer(); };
    undoPLayer.Description = "undoplayer [player] [time] - Undoes the blocks placed by a player in the given duration.";
    undoPLayer.Group = "Building";
    Commands.push_back(undoPLayer);

    Command undoCmd;
    undoCmd.Id = "Undo";
    undoCmd.Name = "undo";
    undoCmd.Internal = true;
    undoCmd.Hidden = false;
    undoCmd.Rank = 0;
    undoCmd.RankShow = 0;
    undoCmd.CanConsole = false;
    undoCmd.Function = [this] { CommandUndo(); };
    undoCmd.Description = "undo [time] - Undoes your block changes for the past [time] seconds.";
    undoCmd.Group = "Building";
    Commands.push_back(undoCmd);

    Command redoCmd;
    redoCmd.Id = "Redo";
    redoCmd.Name = "redo";
    redoCmd.Internal = true;
    redoCmd.Hidden = false;
    redoCmd.Rank = 0;
    redoCmd.RankShow = 0;
    redoCmd.CanConsole = false;
    redoCmd.Function = [this] { CommandRedo(); };
    redoCmd.Description = "redo [time] - Replays block changes you removed with /undo";
    redoCmd.Group = "Building";
    Commands.push_back(redoCmd);

    Command mapList;
    mapList.Id = "List-Maps";
    mapList.Name = "maps";
    mapList.Internal = true;
    mapList.Hidden = false;
    mapList.Rank = 0;
    mapList.RankShow = 0;
    mapList.CanConsole = true;
    mapList.Function = [this] { CommandListMaps(); };
    mapList.Description = "Lists all maps available to change to.";
    mapList.Group = "Common";
    Commands.push_back(mapList);

    Command serverInfo;
    serverInfo.Id = "Server-Info";
    serverInfo.Name = "serverinfo";
    serverInfo.Internal = true;
    serverInfo.Hidden = false;
    serverInfo.Rank = 0;
    serverInfo.RankShow = 0;
    serverInfo.CanConsole = true;
    serverInfo.Function = [this] { CommandServerInfo(); };
    serverInfo.Description = "Displays information about this server.";
    serverInfo.Group = "Common";
    Commands.push_back(serverInfo);

    Command logCommand;
    logCommand.Id = "Log";
    logCommand.Name = "log";
    logCommand.Internal = true;
    logCommand.Hidden = false;
    logCommand.Rank = 260;
    logCommand.RankShow = 260;
    logCommand.CanConsole = false;
    logCommand.Function = [this] { CommandLogLast(); };
    logCommand.Description = "log [lines] - Display the last lines from the server console log.";
    logCommand.Group = "Admin";
    Commands.push_back(logCommand);

    Command tpCommand;
    tpCommand.Id = "Teleport";
    tpCommand.Name = "tp";
    tpCommand.Internal = true;
    tpCommand.Hidden = false;
    tpCommand.Rank = 0;
    tpCommand.RankShow = 0;
    tpCommand.CanConsole = false;
    tpCommand.Function = [this] { CommandTeleport(); };
    tpCommand.Description = "tp [player] - Teleport yourself to another player.";
    tpCommand.Group = "Common";
    Commands.push_back(tpCommand);

    Command bringCommand;
    bringCommand.Id = "Bring";
    bringCommand.Name = "bring";
    bringCommand.Internal = true;
    bringCommand.Hidden = false;
    bringCommand.Rank = 100;
    bringCommand.RankShow = 100;
    bringCommand.CanConsole = false;
    bringCommand.Function = [this] { CommandBring(); };
    bringCommand.Description = "bring [player] - Bring a player to your location.";
    bringCommand.Group = "Common";
    Commands.push_back(bringCommand);

    Command mLoadCommand;
    mLoadCommand.Id = "Map-Load";
    mLoadCommand.Name = "mapload";
    mLoadCommand.Internal = true;
    mLoadCommand.Hidden = false;
    mLoadCommand.Rank = 260;
    mLoadCommand.RankShow = 260;
    mLoadCommand.CanConsole = false;
    mLoadCommand.Function = [this] { CommandLoadMap(); };
    mLoadCommand.Description = "mapload [folder] - Loads a map from a given file into this map file.";
    mLoadCommand.Group = "Map";
    Commands.push_back(mLoadCommand);

    Command mResizeCmd;
    mResizeCmd.Id = "Map-Resize";
    mResizeCmd.Name = "mapresize";
    mResizeCmd.Internal = true;
    mResizeCmd.Hidden = false;
    mResizeCmd.Rank = 150;
    mResizeCmd.RankShow = 150;
    mResizeCmd.CanConsole = false;
    mResizeCmd.Function = [this] { CommandResizeMap(); };
    mResizeCmd.Description = "mapresize [x] [y] [z] - Resizes this map to a new size.";
    mResizeCmd.Group = "Map";
    Commands.push_back(mResizeCmd);

    Command mfillCommand;
    mfillCommand.Id = "Map-Fill";
    mfillCommand.Name = "mapfill";
    mfillCommand.Internal = true;
    mfillCommand.Hidden = false;
    mfillCommand.Rank = 150;
    mfillCommand.RankShow = 150;
    mfillCommand.CanConsole = false;
    mfillCommand.Function = [this] { CommandMapFill(); };
    mfillCommand.Description = "mapfill [gen] [args] - Run a map generator on this map. Args optional.";
    mfillCommand.Group = "Map";
    Commands.push_back(mfillCommand);

    Command mRename;
    mRename.Id = "Map-Rename";
    mRename.Name = "maprename";
    mRename.Internal = true;
    mRename.Hidden = false;
    mRename.Rank = 150;
    mRename.RankShow = 150;
    mRename.CanConsole = false;
    mRename.Function = [this] { CommandRenameMap(); };
    mRename.Description = "maprename [name] - Changes the name of this map in settings and on disk.";
    mRename.Group = "Map";
    Commands.push_back(mRename);

    Command mDelete;
    mDelete.Id = "Map-Delete";
    mDelete.Name = "mapdelete";
    mDelete.Internal = true;
    mDelete.Hidden = false;
    mDelete.Rank = 260;
    mDelete.RankShow = 260;
    mDelete.CanConsole = false;
    mDelete.Function = [this] { CommandDeleteMap(); };
    mDelete.Description = "Delete this map.";
    mDelete.Group = "Map";
    Commands.push_back(mDelete);

    Command mAdd;
    mAdd.Id = "Map-Add";
    mAdd.Name = "mapadd";
    mAdd.Internal = true;
    mAdd.Hidden = false;
    mAdd.Rank = 260;
    mAdd.RankShow = 260;
    mAdd.CanConsole = true;
    mAdd.Function = [this] { CommandAddMap(); };
    mAdd.Description = "mapadd [name] - Creates a new map on the server.";
    mAdd.Group = "Map";
    Commands.push_back(mAdd);

    Command mrbs;
    mrbs.Id = "Map_Rank_Build_Set";
    mrbs.Name = "mapbuildrank";
    mrbs.Internal = true;
    mrbs.Hidden = false;
    mrbs.Rank = 150;
    mrbs.RankShow = 150;
    mrbs.CanConsole = false;
    mrbs.Function = [this] { CommandMapRankBuildSet(); };
    mrbs.Description = "mapbuildrank [rank] - Sets the minimum rank you need to build on this map.";
    mrbs.Group = "Map";
    Commands.push_back(mrbs);

    Command mrss;
    mrss.Id = "Map_Rank_Show_Set";
    mrss.Name = "mapshowrank";
    mrss.Internal = true;
    mrss.Hidden = false;
    mrss.Rank = 150;
    mrss.RankShow = 150;
    mrss.CanConsole = false;
    mrss.Function = [this] { CommandMapRankShowSet(); };
    mrss.Description = "mapshowrank [rank] - Sets the minimum rank you need to have to see this map on /maps.";
    mrss.Group = "Map";
    Commands.push_back(mrss);

    Command mrjs;
    mrjs.Id = "Map_Rank_Join_Set";
    mrjs.Name = "mapjoinrank";
    mrjs.Internal = true;
    mrjs.Hidden = false;
    mrjs.Rank = 150;
    mrjs.RankShow = 150;
    mrjs.CanConsole = false;
    mrjs.Function = [this] { CommandMapRankJoinSet(); };
    mrjs.Description = "mapjoinrank [rank] - Sets the minimum rank to join this map.";
    mrjs.Group = "Map";
    Commands.push_back(mrjs);

    Command mStopPhys;
    mStopPhys.Id = "Map_Physic_Stop";
    mStopPhys.Name = "pstop";
    mStopPhys.Internal = true;
    mStopPhys.Hidden = false;
    mStopPhys.Rank = 150;
    mStopPhys.RankShow = 150;
    mStopPhys.CanConsole = false;
    mStopPhys.Function = [this] { CommandStopPhysics(); };
    mStopPhys.Description = "Stops block physics from being processed on this map.";
    mStopPhys.Group = "Map";
    Commands.push_back(mStopPhys);

    Command mStartPhys;
    mStartPhys.Id = "Map_Physic_Start";
    mStartPhys.Name = "pstart";
    mStartPhys.Internal = true;
    mStartPhys.Hidden = false;
    mStartPhys.Rank = 150;
    mStartPhys.RankShow = 150;
    mStopPhys.CanConsole = false;
    mStartPhys.Function = [this] { CommandStartPhysics(); };
    mStartPhys.Description = "Enables block physics on this map.";
    mStartPhys.Group = "Map";
    Commands.push_back(mStartPhys);

    Command mSetSpawn;
    mSetSpawn.Id = "Set-Spawn";
    mSetSpawn.Name = "setspawn";
    mSetSpawn.Internal = true;
    mSetSpawn.Hidden = false;
    mSetSpawn.Rank = 150;
    mSetSpawn.RankShow = 150;
    mSetSpawn.CanConsole = false;
    mSetSpawn.Function = [this] { CommandSetSpawn(); };
    mSetSpawn.Description = "Sets the map spawn point to your current location.";
    mSetSpawn.Group = "Map";
    Commands.push_back(mSetSpawn);

    Command mSetKillSpawn;
    mSetKillSpawn.Id = "Set-Killspawn";
    mSetKillSpawn.Name = "setkillspawn";
    mSetKillSpawn.Internal = true;
    mSetKillSpawn.Hidden = false;
    mSetKillSpawn.Rank = 150;
    mSetKillSpawn.RankShow = 150;
    mSetKillSpawn.CanConsole = false;
    mSetKillSpawn.Function = [this] { CommandSetKilLSpawn(); };
    mSetKillSpawn.Description = "Sets the respawn location of the map to your current location.";
    mSetKillSpawn.Group = "Map";
    Commands.push_back(mSetKillSpawn);

    Command mTeleporters;
    mTeleporters.Id = "List-Teleporters";
    mTeleporters.Name = "teleporters";
    mTeleporters.Internal = true;
    mTeleporters.Hidden = false;
    mTeleporters.Rank = 0;
    mTeleporters.RankShow = 0;
    mTeleporters.CanConsole = false;
    mTeleporters.Function = [this] { CommandTeleporters(); };
    mTeleporters.Description = "Lists all teleporters on this map.";
    mTeleporters.Group = "Map";
    Commands.push_back(mTeleporters);

    Command cDeleteTp;
    cDeleteTp.Id = "Delete-Teleporterbox";
    cDeleteTp.Name = "deltp";
    cDeleteTp.Internal = true;
    cDeleteTp.Hidden = false;
    cDeleteTp.Rank = 150;
    cDeleteTp.RankShow = 150;
    cDeleteTp.CanConsole = false;
    cDeleteTp.Function = [this] { CommandDeleteTeleporter(); };
    cDeleteTp.Description = "deltp [name] - Deletes a teleporter from this map.";
    cDeleteTp.Group = "Map";
    Commands.push_back(cDeleteTp);

    Command mapInfo;
    mapInfo.Id = "Map-Info";
    mapInfo.Name = "mapinfo";
    mapInfo.Internal = true;
    mapInfo.Hidden = false;
    mapInfo.Rank = 0;
    mapInfo.RankShow = 0;
    mapInfo.CanConsole = false;
    mapInfo.Function = [this] { CommandMapInfo(); };
    mapInfo.Description = "Displays information about the current map.";
    mapInfo.Group = "Map";
    Commands.push_back(mapInfo);

    Command usermaps;
    usermaps.Id = "List-Usermaps";
    usermaps.Name = "usermaps";
    usermaps.Internal = true;
    usermaps.Hidden = false;
    usermaps.Rank = 150;
    usermaps.RankShow = 150;
    usermaps.CanConsole = true;
    usermaps.Function = [this] { CommandUserMaps(); };
    usermaps.Description = "Display a list of saved builds for import.";
    usermaps.Group = "Building";
    Commands.push_back(usermaps);

    Command placeCmd;
    placeCmd.Id = "Place";
    placeCmd.Name = "place";
    placeCmd.Internal = true;
    placeCmd.Hidden = false;
    placeCmd.Rank = 0;
    placeCmd.RankShow = 0;
    placeCmd.CanConsole = false;
    placeCmd.Function = [this] { CommandPlace(); };
    placeCmd.Description = "Places a block below your feet. Default of whatever block you placed last.";
    placeCmd.Group = "Building";
    Commands.push_back(placeCmd);

    Save();
}
void CommandMain::RefreshGroups() {
    // -- Build list of groups.
    CommandGroups.clear();
    for(auto &cmd : Commands) {
        if (!cmd.Group.empty()) {
            bool found = false;
            for(auto &grp : CommandGroups) {
                if (Utils::InsensitiveCompare(grp.Name, cmd.Group)) {
                    found = true;
                    if (grp.RankShow > cmd.Rank && cmd.Rank >= cmd.RankShow) {
                        grp.RankShow = cmd.Rank;
                    }
                    if (grp.RankShow > cmd.RankShow && cmd.RankShow >= cmd.Rank) {
                        grp.RankShow = cmd.RankShow;
                    }
                    break;
                }
            }
            if (!found) {
                CommandGroup newCg;
                newCg.RankShow = 32757;
                if (newCg.RankShow > cmd.Rank && cmd.Rank >= cmd.RankShow) {
                    newCg.RankShow = cmd.Rank;
                }
                if (newCg.RankShow > cmd.RankShow && cmd.RankShow >= cmd.Rank) {
                    newCg.RankShow = cmd.RankShow;
                }
                newCg.Name = cmd.Group;
                CommandGroups.push_back(newCg);
            }
        }
    }
}

void CommandMain::Load() {
    std::string cmdFilename = Files::GetFile(COMMAND_FILENAME);

    if (Utils::FileSize(cmdFilename) == -1) {
        Logger::LogAdd(MODULE_NAME, "Commands file does not exist, will generate.", L_ERROR, GLF);
        Init();
        SaveFile = true;
        return;
    }

    PreferenceLoader pl(cmdFilename, "");
    pl.LoadFile();
    Commands.clear();
    Init();

    // -- Iterate everything in the commands file to load.
    for (auto const &si : pl.SettingsDictionary) {
        if (si.first.empty())
            continue;

        bool found = false;

        for(auto &cmd : Commands) {
            if (cmd.Id == si.first && !cmd.Hidden) {
                found = true;
                pl.SelectGroup(si.first);
                cmd.Name = pl.Read("Name", "");
                cmd.Rank = pl.Read("Rank", 0);
                cmd.RankShow = pl.Read("Rank_Show", 0);
                cmd.Plugin = pl.Read("Plugin", "");
                cmd.Group = pl.Read("Group", "");
                cmd.Description = pl.Read("Description", "-");
                cmd.Hidden = pl.Read("Hidden", false);
                break;
            }
        }

        if (!found) {
            Command newCmd;
            newCmd.Id = si.first;
            pl.SelectGroup(si.first);
            newCmd.Name = pl.Read("Name", "");
            newCmd.Rank = pl.Read("Rank", 0);
            newCmd.RankShow = pl.Read("Rank_Show", 0);
            newCmd.Plugin = pl.Read("Plugin", "");
            newCmd.Group = pl.Read("Group", "");
            newCmd.Description = pl.Read("Description", "-");
            newCmd.Hidden = false;
            newCmd.Internal = false;
            
            Commands.push_back(newCmd);
        }
    }

    RefreshGroups();

    Logger::LogAdd(MODULE_NAME, "File loaded [" + cmdFilename + "]", NORMAL, GLF);
    time_t modTime = Utils::FileModTime(cmdFilename);
    FileDateLast = modTime;
}

void CommandMain::Save() {
    if (!SaveFile) { return; }
    std::string cmdFilename = Files::GetFile(COMMAND_FILENAME);

    PreferenceLoader pl(cmdFilename, "");
    for (auto &cmd : Commands) {
        pl.SelectGroup(cmd.Id);
        pl.Write("Name", cmd.Name);
        pl.Write("Rank", cmd.Rank);
        pl.Write("Rank_Show", cmd.RankShow);
        pl.Write("Plugin", cmd.Plugin);
        pl.Write("Description", cmd.Description);
        pl.Write("Hidden", cmd.Hidden);
        pl.Write("Internal", cmd.Internal);
        pl.Write("Group", cmd.Group);
    }

    pl.SaveFile();
    Logger::LogAdd(MODULE_NAME, "File saved [" + cmdFilename + "]", NORMAL, GLF);
    SaveFile = false;
}

void CommandMain::MainFunc() {
    std::string blockFile = Files::GetFile(COMMAND_FILENAME);
    time_t modTime = Utils::FileModTime(blockFile);

    if (modTime != FileDateLast) {
        Load();
        FileDateLast = modTime;
    }

    if (SaveFile) {
        Save();
    }
}

CommandMain* CommandMain::GetInstance() {
    if (Instance == nullptr)
        Instance = new CommandMain();

    return Instance;
}

void CommandMain::CommandDo(const std::shared_ptr<IMinecraftClient>& client, const std::string& input) {
    CommandClientId = client->GetId();
    std::vector<std::string> splitString = Utils::splitString(input);
    
    if (splitString.empty())
        return;
    
    ParsedCommand = splitString[0];

    for (int i = 0; i < COMMAND_OPERATORS_MAX; i++) {
        ParsedOperator[i] = "";
        if (i+1 < splitString.size()) {
            ParsedOperator.at(i) = splitString[i + 1];
        }
    }

    if (input.size() > ParsedCommand.size())
        ParsedText0 = input.substr(ParsedCommand.size()+1);
    else
    {
        ParsedText0 = "";
    }

    if (ParsedText0.find(' ') > 0) {
        ParsedText1 = ParsedText0.substr(ParsedText0.find(' ')+1);
    } else {
        ParsedText1 = "";
    }

    bool found = false;
    for(const auto& cmd : Commands) {
        if (!Utils::InsensitiveCompare(cmd.Name, ParsedCommand))
            continue;

        if (client->GetId() == -200 && !cmd.CanConsole) {
            Logger::LogAdd(MODULE_NAME, "&cThis command is not accessible via console!", L_ERROR, GLF);
            found = true;
        } else if (client->GetRank() < cmd.Rank) {
            client->SendChat("§EYou are not allowed to use this command.");
            found = true;
        } else {
            if (!cmd.Hidden) {
                Logger::LogAdd(MODULE_NAME, "Player '" + client->GetLoginName() + "' used command /" + ParsedCommand + " (" + join(ParsedOperator.begin(), ParsedOperator.end()) + ")", COMMAND, __FILE__, __LINE__, __FUNCTION__);
            }
            if (!cmd.Plugin.empty()) {
                // -- Run plugin based command
                D3PP::plugins::PluginManager *pm = D3PP::plugins::PluginManager::GetInstance();
                std::string functionName = cmd.Plugin;
                Utils::replaceAll(functionName, "Lua:", "");
                pm->TriggerCommand(functionName, client->GetId(), ParsedCommand, ParsedText0, ParsedText1, ParsedOperator.at(0), ParsedOperator.at(1), ParsedOperator.at(2), ParsedOperator.at(3), ParsedOperator.at(4));

            } else if (cmd.Function != nullptr) {
                cmd.Function();
            }
            found = true;
        }
    }
    if (!found) {
        client->SendChat("§ECan't find the command '" + ParsedCommand + "'.");
    }
}

void CommandMain::CommandCommands() {
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);

    if (c == nullptr)
        return;

    std::string groupName = ParsedOperator.at(0);
    short playerRank = c->GetRank();
    std::string allString = "all";

    if (groupName.empty()) {
       c->SendChat("§SCommand Groups:"); 
       c->SendChat("§S/commands all");
       
       for(auto const &cg : CommandGroups) {
           if (cg.RankShow <= playerRank) {
               c->SendChat("§S/commands " + cg.Name);
           }
       }

       return;
    }

    bool found = false;
    for(auto  &cg : CommandGroups) {
        if (Utils::InsensitiveCompare(groupName, cg.Name) || Utils::InsensitiveCompare(groupName, allString)) {
            c->SendChat("§SCommands:"); 
            std::string textToSend;
            for(auto &cm : Commands) {
                if ((cm.RankShow > playerRank) || (cm.Rank > playerRank) || (cm.Hidden) || (!Utils::InsensitiveCompare(cm.Group, groupName) && !Utils::InsensitiveCompare(groupName, allString)))
                    continue;
                std::string textAdd = cm.Name + " &3| &f";
                if (64 - textToSend.size() >= textAdd.size())
                    textToSend += textAdd;
                else {
                    c->SendChat(textToSend);
                    textToSend = textAdd;
                }
            }
            if (!textToSend.empty()) {
                c->SendChat(textToSend);
            }
            found = true;
            break;
        }
    }

    if (!found) {
        c->SendChat("§SCan't find command group '" + groupName + "'");
    }
}

void CommandMain::CommandHelp() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);

    bool found = false;
    for(const auto& cmd : Commands) {
        if (!Utils::InsensitiveCompare(cmd.Name, ParsedOperator.at(0)))
            continue;

        c->SendChat("§SCommand Help:");
        c->SendChat(cmd.Description);
        found = true;
        break;
    }

    if (!found) {
        c->SendChat("§ECan't find command '" + ParsedOperator.at(0) + "'");
    }
}

void CommandMain::CommandPlayers() {
    auto c = Network::GetClient(CommandClientId);
    if (!c) return;

    c->SendChat("§SPlayers:");
    std::string textToSend;

    std::shared_lock lock(D3PP::network::Server::roMutex, std::defer_lock);

    auto sendBuffer = [&](const std::string& buffer) {
        if (!buffer.empty()) c->SendChat(buffer);
    };

    for (const auto& nc : D3PP::network::Server::roClients) {
        auto entity = nc && nc->GetPlayerInstance() ? nc->GetPlayerInstance()->GetEntity() : nullptr;
        if (entity && entity->playerList) {
            std::string playerName = Entity::GetDisplayname(entity->Id);
            std::string toAdd = playerName + " &c| ";
            if (textToSend.size() + toAdd.size() <= 64) {
                textToSend += toAdd;
            } else {
                sendBuffer(textToSend);
                textToSend = toAdd;
            }
        }
    }
}


void CommandMain::CommandPlayerInfo() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    Rank* rm = Rank::GetInstance();

    auto ple = pll->GetPointer(ParsedOperator.at(0));
    if (ple == nullptr) {
        c->SendChat("§ECan't find a player named '" + ParsedOperator.at(0) + "'");
        return;
    }
    RankItem ri = rm->GetRank(ple->PRank, false);
    std::string textTosend = "§SPlayer Info: <br>";
    textTosend += "§SNumber: " + stringulate(ple->Number) + "<br>";
    textTosend += "§SName: " + ple->GetDisplayName() + "<br>";
    textTosend += "§SRank: " + ri.Name + " (" + stringulate(ple->PRank) + ")<br>";
    textTosend += "§SIP: " + ple->IP + "<br>";
    textTosend += "§SOntime: " + stringulate(ple->OntimeCounter/3600.0) + "h<br>";
    textTosend += "§SLogins: " + stringulate(ple->LoginCounter) + "<br>";
    textTosend += "§SKicks: " + stringulate(ple->KickCounter) + "<br>";
    
    if (ple->Banned) {
        textTosend += "&4Player is banned<br>";
    }
    if (ple->Stopped) {
        textTosend += "&4Player is stopped<br>";
    }
    if (ple->MuteTime > time(nullptr)) {
        textTosend += "&4Player is muted<br>";
    }
    c->SendChat(textTosend);
}

void CommandMain::CommandChangeRank() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();

    std::string playerName = ParsedOperator.at(0);

    if (!Utils::IsNumeric(ParsedOperator.at(1))) {
        c->SendChat("§EThe second parameter should be a number!");
        return;
    }

    int rankVal = std::stoi(ParsedOperator.at(1));
    std::string reason = ParsedText2;

    if (rankVal >= -32768 && rankVal <= 32767) {
        auto ple = pll->GetPointer(ParsedOperator.at(0));
        if (ple == nullptr) {
            c->SendChat("§ECan't find a player named '" + ParsedOperator.at(0) + "'");
            return;
        }
        if (c->GetRank() <= ple->PRank) {
            c->SendChat("§ECan't modify the rank of someone higher than you");
            return;
        }
        if (c->GetRank() <= rankVal) {
            c->SendChat("§ECan't set a rank higher than your own");
            return;
        }
        ple->SetRank(rankVal, reason);
        c->SendChat("§SPlayers rank was updated.");
    }
}

void CommandMain::CommandGlobal() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);

    if (c == nullptr)
        return;

    const std::string onString = "on";
    const std::string trueString = "true";
    const std::string offString = "off";
    const std::string falseString = "false";

    if (Utils::InsensitiveCompare(ParsedOperator.at(0), onString) || Utils::InsensitiveCompare(ParsedOperator.at(0), trueString)) {
        c->SetGlobalChat(true);
        c->SendChat("§SGlobal chat is now on by default.");
    } else if (Utils::InsensitiveCompare(ParsedOperator.at(0), offString) || Utils::InsensitiveCompare(ParsedOperator.at(0), falseString)) {
        c->SetGlobalChat(false);
        c->SendChat("§SGlobal chat is now off by default.");
    } else {
        c->SetGlobalChat(!c->GetGlobalChat());
        std::string status = (c->GetGlobalChat() ? "on" : "off");
        c->SendChat("§SGlobal chat is now " + status + " by default.");
    }
}

void CommandMain::CommandPing() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);

    if (c == nullptr)
        return;

    c->SendChat("§SYour ping is " + stringulate(c->GetPing()) + "s.");
}

void CommandMain::CommandChangeMap() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);

    if (c == nullptr)
        return;

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> mi = mm->GetPointer(ParsedText0);
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);

    if (mi != nullptr) {
        MapPermissions perms = mi->GetMapPermissions();
        if (perms.RankJoin > c->GetRank()) {
            Entity::MessageToClients(clientEntity->Id, "§EYou are not allowed to join map '" + mi->Name() + "'");
            return;
        }

        auto concrete = std::static_pointer_cast<D3PP::world::Player>(c->GetPlayerInstance());
        concrete->ChangeMap(mi);

        clientEntity->PositionSet(mi->ID, mi->GetSpawn(), 255, true);

    } else {
        c->SendChat("§EUnable to find map '" + ParsedText0 + "'.");
    }
}

void CommandMain::CommandSaveMap() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    std::shared_ptr<Entity> e = Entity::GetPointer(CommandClientId, true);

    if (e == nullptr)
        return;

    MapMain* mm = MapMain::GetInstance();
    std::string mapDirectory;

    if (!ParsedText0.empty()) {
        mapDirectory = Files::GetFolder("Maps") + ParsedText0;
        if (mapDirectory.substr(mapDirectory.size()-2, 1) != "/") {
            mapDirectory += "/";
        }
    }

    mm->AddSaveAction(CommandClientId, e->MapID, mapDirectory);
    c->SendChat("§SSave Queued.");
}

void CommandMain::CommandGetRank() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    Player_List* pll = Player_List::GetInstance();
    Rank* rm = Rank::GetInstance();
    std::shared_ptr<PlayerListEntry> ple;
    
    if (ParsedOperator.at(0).empty()) {
        ple = clientEntity->playerList;
    } else {
        ple = pll->GetPointer(ParsedOperator.at(0));
    }
    
    if (ple == nullptr) {
        c->SendChat("§ECan't find a player named '" + ParsedOperator.at(0) + "'");
        return;
    }

    RankItem ri = rm->GetRank(ple->PRank, false);
    std::string textTosend = "§SPlayer '" + ple->Name + "' is ranked '" + ri.Prefix + ri.Name + ri.Suffix + "'.";

    c->SendChat(textTosend);
}

void CommandMain::CommandSetMaterial() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    Block* bm = Block::GetInstance();

    if (ParsedText0.empty()) {
        clientEntity->buildMaterial = -1;
        c->SendChat("§SMaterial reset.");
        return;
    }

    MapBlock b = bm->GetBlock(ParsedText0);
    if (b.Id == -1) {
        c->SendChat("§ECan't find a block called '" + ParsedText0 + "'.");
        return;
    }
    clientEntity->buildMaterial = b.Id;
    c->SendChat("§SYour build material is now " + b.Name);
}

void CommandMain::CommandKick() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    std::shared_ptr<PlayerListEntry> ple;

    ple = pll->GetPointer(ParsedOperator.at(0));
    if (ple == nullptr) {
        c->SendChat("§ECan't find a player named '" + ParsedOperator.at(0) + "'");
        return;
    }
    std::string kickReason = (ParsedText1.empty() ? "§SYou were kicked." : ParsedText1);

    if (c->GetRank() > ple->PRank) {
        ple->Kick(kickReason, true);
    } else {
        c->SendChat("§ECan't kick someone ranked higher than you.");
        return;
    }
}

void CommandMain::CommandBan() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    std::shared_ptr<PlayerListEntry> ple;

    ple = pll->GetPointer(ParsedOperator.at(0));
    if (ple == nullptr) {
        c->SendChat("§ECan't find a player named '" + ParsedOperator.at(0) + "'");
        return;
    }
    std::string banReason = (ParsedText1.empty() ? "§SYou were banned." : ParsedText1);
    if (c->GetRank() > ple->PRank) {
        ple->Ban(banReason);
    } else {
        c->SendChat("§ECan't ban someone ranked higher than you.");
        return;
    }
}

void CommandMain::CommandUnban() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    std::shared_ptr<PlayerListEntry> ple;

    ple = pll->GetPointer(ParsedOperator.at(0));
    if (ple == nullptr) {
        c->SendChat("§ECan't find a player named '" + ParsedOperator.at(0) + "'");
        return;
    }
    if (c->GetRank() > ple->PRank) {
        ple->Unban();
    } else {
        c->SendChat("§ECan't ban someone ranked higher than you.");
        return;
    }
}

void CommandMain::CommandStop() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    std::shared_ptr<PlayerListEntry> ple;

    ple = pll->GetPointer(ParsedOperator.at(0));
    if (ple == nullptr) {
        c->SendChat("§ECan't find a player named '" + ParsedOperator.at(0) + "'");
        return;
    }
    std::string stopReason = (ParsedText1.empty() ? "§SYou were stopped." : ParsedText1);
    if (c->GetRank() > ple->PRank) {
        ple->Stop(stopReason);
    } else {
        c->SendChat("§ECan't stop someone ranked higher than you.");
        return;
    }
}

void CommandMain::CommandUnStop() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    std::shared_ptr<PlayerListEntry> ple;

    ple = pll->GetPointer(ParsedOperator.at(0));
    if (ple == nullptr) {
        c->SendChat("§ECan't find a player named '" + ParsedOperator.at(0) + "'");
        return;
    }
    if (c->GetRank() > ple->PRank) {
        ple->Unstop();
    } else {
        c->SendChat("§ECan't stop someone ranked higher than you.");
        return;
    }
}

void CommandMain::CommandMute() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    std::shared_ptr<PlayerListEntry> ple;

    ple = pll->GetPointer(ParsedOperator.at(0));
    if (ple == nullptr) {
        c->SendChat("§ECan't find a player named '" + ParsedOperator.at(0) + "'");
        return;
    }
    if ((ParsedOperator.at(1).empty())) {
        ParsedOperator.at(1) = "99999";
    }

    if (c->GetRank() > ple->PRank) {
        ple->Mute(stoi(ParsedOperator.at(1)), "Muted by " + c->GetLoginName());
    } else {
        c->SendChat("§ECan't mute someone ranked higher than you.");
        return;
    }
}

void CommandMain::CommandUnmute() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    Rank* rm = Rank::GetInstance();
    std::shared_ptr<PlayerListEntry> ple;

    ple = pll->GetPointer(ParsedOperator.at(0));
    if (ple == nullptr) {
        c->SendChat("§ECan't find a player named '" + ParsedOperator.at(0) + "'");
        return;
    }
    if (c->GetRank() > ple->PRank) {
        ple->Unmute();
    }else {
        c->SendChat("§ECan't unmute someone ranked higher than you.");
        return;
    }
}

void CommandMain::CommandMaterials() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    c->SendChat("§SMaterials:");
    Block* bm = Block::GetInstance();
    std::string toSend;
    for (int i = 0; i < 255; ++i) {
        MapBlock block = bm->GetBlock(i);
        std::string toAdd;
        if (block.Special && block.RankPlace <= c->GetRank()) {
            toAdd += "§S" + block.Name + " &f| ";
            if (64 - toSend.size() >= toAdd.size())
                toSend += toAdd;
            else {
                c->SendChat(toSend);
                toSend = toAdd;
            }

        }
    }
    if (!toSend.empty()) {
        c->SendChat(toSend);
    }
}

void CommandMain::CommandListMaps() const {
    
    MapMain* mapMain = MapMain::GetInstance();

    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    c->SendChat("§SMaps:");
    std::string toSend;
    for(auto const &map : mapMain->_maps) {
        std::string toAdd;
        MapPermissions perms = map.second->GetMapPermissions();
        if (perms.RankShow <= c->GetRank()) {
            toAdd += "§S" + map.second->Name() + " &f| ";
            if (64 - toSend.size() >= toAdd.size())
                toSend += toAdd;
            else {
                c->SendChat(toSend);
                toSend = toAdd;
            }

        }
    }
    if (!toSend.empty()) {
        c->SendChat(toSend);
    }
}

void CommandMain::CommandServerInfo() const {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    std::string serverRunTime = stringulate(time(nullptr) - System::startTime / 120.0);

    c->SendChat("§SServer Info:");
#ifdef __linux__
    c->SendChat("§SD3PP v" + stringulate(SYSTEM_VERSION_NUMBER) + ", Linux (x64)");
#else
#ifdef MSVC
    c->SendChat("§SD3PP v" + stringulate(SYSTEM_VERSION_NUMBER) + ", Windows [MSVC] (x86)");
#else
    c->SendChat("§SD3PP v" + stringulate(SYSTEM_VERSION_NUMBER) + ", Windows (x64)");
#endif
#endif
    c->SendChat("§SRun time: " + serverRunTime + "h");
}

void CommandMain::CommandLogLast() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);

    int numLines = 10;

    if (!ParsedOperator.at(0).empty()) {
        numLines = stoi(ParsedOperator.at(0));
    }
    c->SendChat("§SLog:");
    Logger* logMain = Logger::GetInstance();
    if (numLines > logMain->Messages.size()) {
        numLines = logMain->Messages.size();
    }
    int logSize = logMain->Messages.size();
    for(int i = 0; i< numLines; i++) {
        LogMessage message = logMain->Messages.at(logSize-i-1);
        c->SendChat(" " + message.Message);
    }
}


void CommandMain::CommandUndoPlayer() {
    
    Player_List* playerList= Player_List::GetInstance();
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    std::shared_ptr<Entity> e = Entity::GetPointer(CommandClientId, true);
    auto entry= playerList->GetPointer(ParsedOperator.at(0));
    
    if (entry == nullptr) {
        c->SendChat("§EUnable to find a player named '" + ParsedOperator.at(0) + "'.");
        return;
    }

    int timeAmount = 30000;

    if (!ParsedOperator.at(1).empty()) {
        timeAmount = stoi(ParsedOperator.at(1));
    }

    c->SendChat("§SBlock changes undone.");
}

void CommandMain::CommandUndo() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    int timeAmount = 30000;

    if (!ParsedOperator.at(0).empty()) {
        timeAmount = stoi(ParsedOperator.at(0));
    }

    c->Undo(timeAmount);
    c->SendChat("§SBlock changes undone.");
}

void CommandMain::CommandRedo() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    int timeAmount = 30000;

    if (!ParsedOperator.at(0).empty()) {
        timeAmount = stoi(ParsedOperator.at(0));
    }

    c->Redo(timeAmount);
    c->SendChat("§SBlock changes Re-done.");
}

void CommandMain::CommandTeleport() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    std::shared_ptr<Entity> entry = Entity::GetPointer(ParsedOperator.at(0));

    if (entry == nullptr) {
        c->SendChat("§EUnable to find a player named '" + ParsedOperator.at(0) + "'.");
        return;
    }

    clientEntity->PositionSet(entry->MapID, entry->Location, 10, true);
}

void CommandMain::CommandBring() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    std::shared_ptr<Entity> entry = Entity::GetPointer(ParsedOperator.at(0));

    if (entry == nullptr) {
        c->SendChat("§EUnable to find a player named '" + ParsedOperator.at(0) + "'.");
        return;
    }

    entry->PositionSet(clientEntity->MapID, clientEntity->Location, 10, true);
}

void CommandMain::CommandMapFill() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    std::shared_ptr<Entity> e = Entity::GetPointer(CommandClientId, true);
    if (!ParsedOperator.at(0).empty()) {
        MapMain* mapMain = MapMain::GetInstance();
        mapMain->AddFillAction(c->GetId(), e->MapID, ParsedOperator.at(0), ParsedText1);
    } else {
        c->SendChat("§EPlease define a function.");
    }
}

void CommandMain::CommandLoadMap() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    std::shared_ptr<Entity> e = Entity::GetPointer(CommandClientId, true);
    std::string mapDirectory;

    if (!ParsedText0.empty()) {
        mapDirectory = Files::GetFolder("Maps") + ParsedText0;
    }

    MapMain* mapMain = MapMain::GetInstance();
    mapMain->AddLoadAction(CommandClientId, e->MapID, mapDirectory);
    c->SendChat("§SLoad added to queue.");
}

void CommandMain::CommandResizeMap() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    std::shared_ptr<Entity> e = Entity::GetPointer(CommandClientId, true);
    if (ParsedOperator.at(0).empty() || ParsedOperator.at(1).empty() || ParsedOperator[2].empty()) {
        c->SendChat("§EPlease provide an X, Y, and Z value.");
        return;
    }
    int X = 0;
    int Y = 0;
    int Z = 0;

    try {
        X = stoi(ParsedOperator.at(0));
        Y = stoi(ParsedOperator.at(1));
        Z= stoi(ParsedOperator.at(2));
    } catch (const std::exception &ex) {
        c->SendChat("§EPlease provide an integer X, Y, and Z value.");
        return;
    } catch (...) {
        c->SendChat("§EPlease provide an integer X, Y, and Z value.");
        return;
    }

    MapMain* mapMain = MapMain::GetInstance();
    mapMain->AddResizeAction(CommandClientId, e->MapID, X, Y, Z);
    c->SendChat("§SResize added to queue.");
}

void CommandMain::CommandRenameMap() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);

    if (ParsedText0.empty()) {
        c->SendChat("§EPlease provide a new name.");
        return;
    }
    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    std::shared_ptr<Map> cMap = mapMain->GetPointer(clientEntity->MapID);
    //cMap->Name() = ParsedText0;
    mapMain->SaveFile = true;
    c->SendChat("§SMap Renamed.");
}

void CommandMain::CommandDeleteMap() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    std::shared_ptr<Entity> e = Entity::GetPointer(CommandClientId, true);
    MapMain* mapMain = MapMain::GetInstance();
    mapMain->AddDeleteAction(CommandClientId, e->MapID);
    c->SendChat("§SDelete Queued.");
}

void CommandMain::CommandAddMap() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);

    if (ParsedText0.empty()) {
        c->SendChat("§EPlease provide a name for the map.");
        return;
    }
    MapMain* mapMain = MapMain::GetInstance();
    mapMain->Add(-1, 64, 64, 64, ParsedText0);
    c->SendChat("§SMap created.");
}

void CommandMain::CommandMapRankBuildSet() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);

    if (ParsedOperator.at(0).empty()) {
        c->SendChat("§EPlease provide a rank value.");
        return;
    }
    int mapBuildRank = 0;

    try {
        mapBuildRank = stoi(ParsedOperator.at(0));
    } catch (const std::exception &ex) {
        c->SendChat("§EPlease provide an integer value.");
        return;
    } catch (...) {
        c->SendChat("§EPlease provide an integer value.");
        return;
    }
    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    std::shared_ptr<Map> cMap = mapMain->GetPointer(clientEntity->MapID);

    MapPermissions currentPerms = cMap->GetMapPermissions();
    currentPerms.RankBuild = mapBuildRank;
    cMap->SetMapPermissions(currentPerms);

    c->SendChat("§SRank updated.");
}

void CommandMain::CommandMapRankJoinSet() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);

    if (ParsedOperator.at(0).empty()) {
        c->SendChat("§EPlease provide a rank value.");
        return;
    }
    int mapJoinRank = 0;

    try {
        mapJoinRank = stoi(ParsedOperator.at(0));
    } catch (const std::exception &ex) {
        c->SendChat("§EPlease provide an integer value.");
        return;
    } catch (...) {
        c->SendChat("§EPlease provide an integer value.");
        return;
    }
    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    std::shared_ptr<Map> cMap = mapMain->GetPointer(clientEntity->MapID);

    MapPermissions currentPerms = cMap->GetMapPermissions();
    currentPerms.RankJoin = mapJoinRank;
    cMap->SetMapPermissions(currentPerms);

    c->SendChat("§SRank updated.");
}

void CommandMain::CommandMapRankShowSet() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);

    if (ParsedOperator.at(0).empty()) {
        c->SendChat("§EPlease provide a rank value.");
        return;
    }
    int mapShowRank = 0;

    try {
        mapShowRank = stoi(ParsedOperator.at(0));
    } catch (const std::exception &ex) {
        c->SendChat("§EPlease provide an integer value.");
        return;
    } catch (...) {
        c->SendChat("§EPlease provide an integer value.");
        return;
    }
    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    std::shared_ptr<Map> cMap = mapMain->GetPointer(clientEntity->MapID);

    MapPermissions currentPerms = cMap->GetMapPermissions();
    currentPerms.RankShow = mapShowRank;
    cMap->SetMapPermissions(currentPerms);

    c->SendChat("§SRank updated.");
}

void CommandMain::CommandStopPhysics() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    std::shared_ptr<Map> cMap = mapMain->GetPointer(clientEntity->MapID);
    cMap->PhysicsStopped = true;
    c->SendChat("§SPhysics stopped.");
}

void CommandMain::CommandStartPhysics() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    std::shared_ptr<Map> cMap = mapMain->GetPointer(clientEntity->MapID);
    cMap->PhysicsStopped = false;
    c->SendChat("§SPhysics started.");
}

void CommandMain::CommandSetSpawn() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    std::shared_ptr<Map> cMap = mapMain->GetPointer(clientEntity->MapID);
    cMap->SetSpawn(clientEntity->Location);
    c->SendChat("§SSpawn updated.");
}

void CommandMain::CommandSetKilLSpawn() {
    

    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    std::shared_ptr<Map> cMap = mapMain->GetPointer(clientEntity->MapID);
    cMap->SetSpawn(clientEntity->Location);
    c->SendChat("§SKill Spawn updated.");
}

void CommandMain::CommandTeleporters() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    std::shared_ptr<Map> cMap = mapMain->GetPointer(clientEntity->MapID);

    c->SendChat("§STeleporters:");
    std::string text;
//    for(auto const &tp : cMap->data.Teleporter) {
//        std::string textAdd = "§S" + tp.first + " &f| ";
//        if (64 - text.size() >= textAdd.size())
//            text+= textAdd;
//        else {
//            c->SendChat(text);
//            text= textAdd;
//        }
//    }
    if (!text.empty()) {
        c->SendChat(text);
    }
}

void CommandMain::CommandDeleteTeleporter() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);

    if (ParsedText0.empty()) {
        c->SendChat("§EPlease provide a name for the teleporter to delete.");
        return;
    }
    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    std::shared_ptr<Map> cMap = mapMain->GetPointer(clientEntity->MapID);
//    bool tpFound = cMap->data.Teleporter.find(ParsedText0) != cMap->data.Teleporter.end();
//    if (tpFound) {
//        cMap->data.Teleporter.erase(ParsedText0);
//        c->SendChat("§STeleporter deleted.");
//    } else {
//        c->SendChat("§STeleporter not found.");
//    }
}

void CommandMain::CommandMapInfo() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    MapMain* mapMain = MapMain::GetInstance();
    Block* bm = Block::GetInstance();
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    std::shared_ptr<Map> cMap = mapMain->GetPointer(clientEntity->MapID);
    std::string textToSend = "§SMapInfo:<br>";
    textToSend += "§SName: " + cMap->Name() + "<br>";
    textToSend += "§SId: " + stringulate(cMap->ID) + "<br>";
    textToSend += "§SDirectory: " + cMap->filePath + "<br>";
    Vector3S mapSize = cMap->GetSize();
    textToSend += "§SSize: " + stringulate(mapSize.X)  + "x" + stringulate(mapSize.Y)  + "x" + stringulate(mapSize.Z) + "<br>";
    MapPermissions perms = cMap->GetMapPermissions();

    textToSend += "§SRanks: Build: " + stringulate(perms.RankBuild) + " Join: " + stringulate(perms.RankJoin) + " Show: " + stringulate(perms.RankShow) + " <br>";
    // Added map texture, edge/side blocks and hack control status
    auto mapEnv = cMap->GetMapEnvironment();
    textToSend += "§STexture URL: " + mapEnv.TextureUrl + "<br>";

    textToSend += "§SEdge Block: " + bm->GetBlock(mapEnv.EdgeBlock).Name + "<br>";
    textToSend += "§SSide Block: " + bm->GetBlock(mapEnv.SideBlock).Name + "<br>";
    textToSend += "§SWeather: " + std::string(mapEnv.WeatherType ? "Rain" : "Clear") + "<br>";

    textToSend += "§SSky Color: " + to_hex_format(mapEnv.SkyColor) + "<br>";
    textToSend += "§SCloud Color: " + to_hex_format(mapEnv.CloudColor) + "<br>";
    textToSend += "§SFog Color: " + to_hex_format(mapEnv.FogColor) + "<br>";
    textToSend += "§ALight Color: " + to_hex_format(mapEnv.Alight) + "<br>";
    textToSend += "§SDLight Color: " + to_hex_format(mapEnv.DLight) + "<br>";


    if (cMap->PhysicsStopped) {
        textToSend += "&cPhysics Stopped&f<br>";
    } else {
        textToSend += "<br>";
    }

    if (cMap->BlockchangeStopped) {
        textToSend += "&Block Changes Stopped&f<br>";
    } else {
        textToSend += "<br>";
    }

    c->SendChat(textToSend);
}

void CommandMain::CommandPlace() {
    
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    MapMain* mapMain = MapMain::GetInstance();
    std::shared_ptr<Entity> clientEntity = Entity::GetPointer(CommandClientId, true);
    std::shared_ptr<Map> cMap = mapMain->GetPointer(clientEntity->MapID);
    Vector3S blockCoords = clientEntity->Location.GetAsBlockCoords();
    if (blockCoords.X< 0) blockCoords.X = 0;
    if (blockCoords.Y< 0) blockCoords.Y= 0;
    if (blockCoords.Z< 0) blockCoords.Z= 0;
    Vector3S mapSize = cMap->GetSize();
    if (blockCoords.X>mapSize.X-1) blockCoords.X = mapSize.X-1;
    if (blockCoords.Y>mapSize.Y-1) blockCoords.Y = mapSize.Y-1;
    if (blockCoords.Z>mapSize.Z-1) blockCoords.Z = mapSize.Z-1;

    bool found = false;
    unsigned char blockToPlace = 0;
    if (ParsedText0.empty()) {
        blockToPlace = clientEntity->lastMaterial;
        found = true;
    } else {
        Block* bm = Block::GetInstance();
        MapBlock block = bm->GetBlock(ParsedText0);
        if (block.Id != -1) {
            found = true;
            blockToPlace = block.Id;
        }
    }

    if (found) {
        cMap->BlockChange(c, blockCoords.X, blockCoords.Y, blockCoords.Z, 1, blockToPlace);
        c->SendChat("§SBlock placed.");
    } else {
        c->SendChat("§ECan't find a block called '" + ParsedText0 + "'.");
    }
}

void CommandMain::CommandUserMaps() {
    std::shared_ptr<IMinecraftClient> c = Network::GetClient(CommandClientId);
    std::string usermapDirectory = Files::GetFolder("Usermaps");

    c->SendChat("§SUsermaps:");
    std::string textToSend;

    if (std::filesystem::is_directory(usermapDirectory)) {
        for (const auto &entry : std::filesystem::directory_iterator(usermapDirectory)) {
            std::string fileName = entry.path().filename().string();

            if (fileName.length() < 3)
                continue;

            if (fileName.substr(fileName.length() - 4) == ".map") {
                textToSend += "§S" + fileName + " &f| ";
            }
        }
    }
    c->SendChat(textToSend);
}
