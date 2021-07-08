#include "Command.h"

#include "common/PreferenceLoader.h"
#include "Network.h"
#include "Player.h"
#include "Player_List.h"
#include "Entity.h"
#include "Block.h"
#include "Map.h"
#include "Rank.h"
#include "Network_Functions.h"
#include "Logger.h"
#include "Files.h"
#include "System.h"
#include "Mem.h"
#include "Utils.h"
#include "Undo.h"

const std::string MODULE_NAME = "Command";
CommandMain* CommandMain::Instance = nullptr;

CommandMain::CommandMain() {
    this->Setup = [this] { Init(); };
    this->Main = [this] { MainFunc(); };
    this->Interval = std::chrono::seconds(1);
    SaveFile = false;
    FileDateLast = 0;
    CommandClientId = -1;
    TaskScheduler::RegisterTask("Commands", *this);
}

void CommandMain::Init() {
    Command kickCommand;
    kickCommand.Id = "Kick";
    kickCommand.Name = "kick";
    kickCommand.Internal = true;
    kickCommand.Hidden = false;
    kickCommand.Rank = 0;
    kickCommand.RankShow = 0;
    kickCommand.Function = [this] { CommandMain::CommandKick(); };
    Commands.push_back(kickCommand);

    Command banCommand;
    banCommand.Id = "Ban";
    banCommand.Name = "ban";
    banCommand.Internal = true;
    banCommand.Hidden = false;
    banCommand.Rank = 0;
    banCommand.RankShow = 0;
    banCommand.Function = [this] { CommandMain::CommandBan(); };
    Commands.push_back(banCommand);

    Command unbanCommand;
    unbanCommand.Id = "Un-Ban";
    unbanCommand.Name = "unban";
    unbanCommand.Internal = true;
    unbanCommand.Hidden = false;
    unbanCommand.Rank = 0;
    unbanCommand.RankShow = 0;
    unbanCommand.Function = [this] { CommandMain::CommandUnban(); };
    Commands.push_back(unbanCommand);

    Command stopcmd;
    stopcmd.Id = "Stop-Player";
    stopcmd.Name = "stop";
    stopcmd.Internal = true;
    stopcmd.Hidden = false;
    stopcmd.Rank = 0;
    stopcmd.RankShow = 0;
    stopcmd.Function = [this] { CommandMain::CommandStop(); };
    Commands.push_back(stopcmd);

    Command unstopCmd;
    unstopCmd.Id = "Un-Stop";
    unstopCmd.Name = "unstop";
    unstopCmd.Internal = true;
    unstopCmd.Hidden = false;
    unstopCmd.Rank = 0;
    unstopCmd.RankShow = 0;
    unstopCmd.Function = [this] { CommandMain::CommandUnStop(); };
    Commands.push_back(unstopCmd);

    Command muteCmd;
    muteCmd.Id = "Mute-Player";
    muteCmd.Name = "mute";
    muteCmd.Internal = true;
    muteCmd.Hidden = false;
    muteCmd.Rank = 0;
    muteCmd.RankShow = 0;
    muteCmd.Function = [this] { CommandMain::CommandMute(); };
    Commands.push_back(muteCmd);

    Command unMuteCmd;
    unMuteCmd.Id = "Unmute-Player";
    unMuteCmd.Name = "unmute";
    unMuteCmd.Internal = true;
    unMuteCmd.Hidden = false;
    unMuteCmd.Rank = 0;
    unMuteCmd.RankShow = 0;
    unMuteCmd.Function = [this] { CommandMain::CommandUnmute(); };
    Commands.push_back(unMuteCmd);

    Command listCommands;
    listCommands.Id = "List-Commands";
    listCommands.Name = "commands";
    listCommands.Internal = true;
    listCommands.Hidden = false;
    listCommands.Rank = 0;
    listCommands.RankShow = 0;
    listCommands.Function = [this] { CommandMain::CommandCommands(); };
    Commands.push_back(listCommands);

    Command helpCommand;
    helpCommand.Id = "Command-Help";
    helpCommand.Name = "cmdhelp";
    helpCommand.Internal = true;
    helpCommand.Hidden = false;
    helpCommand.Rank = 0;
    helpCommand.RankShow = 0;
    helpCommand.Function = [this] { CommandMain::CommandHelp(); };
    Commands.push_back(helpCommand);

    Command listPlayers;
    listPlayers.Id = "List-Players";
    listPlayers.Name = "players";
    listPlayers.Internal = true;
    listPlayers.Hidden = false;
    listPlayers.Rank = 0;
    listPlayers.RankShow = 0;
    listPlayers.Function = [this] { CommandMain::CommandPlayers(); };
    Commands.push_back(listPlayers);

    Command pInfoCmd;
    pInfoCmd.Id = "Player-Info";
    pInfoCmd.Name = "pinfo";
    pInfoCmd.Internal = true;
    pInfoCmd.Hidden = false;
    pInfoCmd.Rank = 0;
    pInfoCmd.RankShow = 0;
    pInfoCmd.Function = [this] { CommandMain::CommandPlayerInfo(); };
    Commands.push_back(pInfoCmd);

    Command pingCommand;
    pingCommand.Id = "Ping";
    pingCommand.Name = "ping";
    pingCommand.Internal = true;
    pingCommand.Hidden = false;
    pingCommand.Rank = 0;
    pingCommand.RankShow = 0;
    pingCommand.Function = [this] { CommandMain::CommandPing(); };
    Commands.push_back(pingCommand);

    Command globalCommand;
    globalCommand.Id = "Global";
    globalCommand.Name = "global";
    globalCommand.Internal = true;
    globalCommand.Hidden = false;
    globalCommand.Rank = 0;
    globalCommand.RankShow = 0;
    globalCommand.Function = [this] { CommandMain::CommandGlobal(); };
    Commands.push_back(globalCommand);

    Command changeMapCommand;
    changeMapCommand.Id = "Map";
    changeMapCommand.Name = "map";
    changeMapCommand.Internal = true;
    changeMapCommand.Hidden = false;
    changeMapCommand.Rank = 0;
    changeMapCommand.RankShow = 0;
    changeMapCommand.Function = [this] { CommandMain::CommandChangeMap(); };
    Commands.push_back(changeMapCommand);

    Command changeRankCommand;
    changeRankCommand.Id = "Set-Rank";
    changeRankCommand.Name = "setrank";
    changeRankCommand.Internal = true;
    changeRankCommand.Hidden = false;
    changeRankCommand.Rank = 0;
    changeRankCommand.RankShow = 0;
    changeRankCommand.Function = [this] { CommandMain::CommandChangeRank(); };
    Commands.push_back(changeRankCommand);

    Command mapSaveCommand;
    mapSaveCommand.Id = "Map-Save";
    mapSaveCommand.Name = "mapsave";
    mapSaveCommand.Internal = true;
    mapSaveCommand.Hidden = false;
    mapSaveCommand.Rank = 0;
    mapSaveCommand.RankShow = 0;
    mapSaveCommand.Function = [this] { CommandMain::CommandSaveMap(); };
    Commands.push_back(mapSaveCommand);

    Command getRankCommand;
    getRankCommand.Id = "Get-Rank";
    getRankCommand.Name = "getrank";
    getRankCommand.Internal = true;
    getRankCommand.Hidden = false;
    getRankCommand.Rank = 0;
    getRankCommand.RankShow = 0;
    getRankCommand.Function = [this] { CommandMain::CommandGetRank(); };
    Commands.push_back(getRankCommand);

    Command setMaterialCommand;
    setMaterialCommand.Id = "Material";
    setMaterialCommand.Name = "material";
    setMaterialCommand.Internal = true;
    setMaterialCommand.Hidden = false;
    setMaterialCommand.Rank = 0;
    setMaterialCommand.RankShow = 0;
    setMaterialCommand.Function = [this] { CommandMain::CommandSetMaterial(); };
    Commands.push_back(setMaterialCommand);

    Command materialList;
    materialList.Id = "List-Materials";
    materialList.Name = "materials";
    materialList.Internal = true;
    materialList.Hidden = false;
    materialList.Rank = 0;
    materialList.RankShow = 0;
    materialList.Function = [this] { CommandMain::CommandMaterials(); };
    Commands.push_back(materialList);

    Command undoTime;
    undoTime.Id = "Undo-Time";
    undoTime.Name = "undotime";
    undoTime.Internal = true;
    undoTime.Hidden = false;
    undoTime.Rank = 0;
    undoTime.RankShow = 0;
    undoTime.Function = [this] { CommandMain::CommandUndoTime(); };
    Commands.push_back(undoTime);

    Command undoPLayer;
    undoPLayer.Id = "Undo-Player";
    undoPLayer.Name = "undoplayer";
    undoPLayer.Internal = true;
    undoPLayer.Hidden = false;
    undoPLayer.Rank = 0;
    undoPLayer.RankShow = 0;
    undoPLayer.Function = [this] { CommandMain::CommandUndoPlayer(); };
    Commands.push_back(undoPLayer);

    Command undoCmd;
    undoCmd.Id = "Undo";
    undoCmd.Name = "undo";
    undoCmd.Internal = true;
    undoCmd.Hidden = false;
    undoCmd.Rank = 0;
    undoCmd.RankShow = 0;
    undoCmd.Function = [this] { CommandMain::CommandUndo(); };
    Commands.push_back(undoCmd);


    Command mapList;
    mapList.Id = "List-Maps";
    mapList.Name = "maps";
    mapList.Internal = true;
    mapList.Hidden = false;
    mapList.Rank = 0;
    mapList.RankShow = 0;
    mapList.Function = [this] { CommandMain::CommandListMaps(); };
    Commands.push_back(mapList);

    Command serverInfo;
    serverInfo.Id = "Server-Info";
    serverInfo.Name = "serverinfo";
    serverInfo.Internal = true;
    serverInfo.Hidden = false;
    serverInfo.Rank = 0;
    serverInfo.RankShow = 0;
    serverInfo.Function = [this] { CommandMain::CommandServerInfo(); };
    Commands.push_back(serverInfo);

    Command logCommand;
    logCommand.Id = "Log";
    logCommand.Name = "log";
    logCommand.Internal = true;
    logCommand.Hidden = false;
    logCommand.Rank = 0;
    logCommand.RankShow = 0;
    logCommand.Function = [this] { CommandMain::CommandLogLast(); };
    Commands.push_back(logCommand);

    Load();
}

void CommandMain::Load() {
    Files* f = Files::GetInstance();
    std::string cmdFilename = f->GetFile(COMMAND_FILENAME);

    if (Utils::FileSize(cmdFilename) == -1) {
        Logger::LogAdd(MODULE_NAME, "Commands file does not exist.", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    PreferenceLoader pl(cmdFilename, "");
    pl.LoadFile();

    std::vector<int> toRemove;
    int i = 0;
    for(auto const &cmd : Commands) {
        if (cmd.Internal == false && pl.SettingsDictionary.find(cmd.Id) == pl.SettingsDictionary.end()) {
            toRemove.push_back(i);
        }
        i++;
    }
    for(auto const &rmi : toRemove) {
        Commands.erase(Commands.begin() + rmi);
    }

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

    // -- Build list of groups.
    CommandGroups.clear();
    for(auto &cmd : Commands) {
        if (cmd.Group != "") {
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

    Logger::LogAdd(MODULE_NAME, "File loaded [" + cmdFilename + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void CommandMain::MainFunc() {

}

CommandMain* CommandMain::GetInstance() {
    if (Instance == nullptr)
        Instance = new CommandMain();

    return Instance;
}

void CommandMain::CommandDo(const std::shared_ptr<NetworkClient> client, std::string input) {
    CommandClientId = client->Id;
    std::vector<std::string> splitString = Utils::splitString(input);
    
    if (splitString.empty())
        return;
    
    ParsedCommand = splitString[0];

    for (int i = 0; i < COMMAND_OPERATORS_MAX; i++) {
        if (i+1 > splitString.size()) {
            ParsedOperator[i] = "";
            break;
        }
        if (i+1 < splitString.size()) {
            ParsedOperator[i] = splitString[i + 1];
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
    for(auto cmd : Commands) {
        if (!Utils::InsensitiveCompare(cmd.Name, ParsedCommand))
            continue;

        if (client->player->tEntity->playerList->PRank < cmd.Rank) {
            NetworkFunctions::SystemMessageNetworkSend(client->Id, "&eYou are not allowed to use this command.");
            found = true;
        } else {
            if (!cmd.Hidden) {
                Logger::LogAdd(MODULE_NAME, "Client '" + client->player->LoginName + "' uses command /" + ParsedCommand, LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
            }
            if (!cmd.Plugin.empty()) {
                // -- Run plugin based command
            } else if (cmd.Function != NULL) {
                cmd.Function();
            }
            found = true;
        }
    }
    if (!found) {
        NetworkFunctions::SystemMessageNetworkSend(client->Id, "&eCan't find the command '" + ParsedCommand + "'.");
    }
}

void CommandMain::CommandCommands() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);

    if (c == nullptr)
        return;

    std::string groupName = ParsedOperator[0];
    short playerRank = c->player->tEntity->playerList->PRank;
    std::string allString = "all";

    if (groupName.empty()) {
       NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCommand Groups:"); 
       NetworkFunctions::SystemMessageNetworkSend(c->Id, "&e/commands all");
       
       for(auto const &cg : CommandGroups) {
           if (cg.RankShow <= playerRank) {
               NetworkFunctions::SystemMessageNetworkSend(c->Id, "&e/commands " + cg.Name);
           }
       }

       return;
    }

    bool found = false;
    for(auto  &cg : CommandGroups) {
        if (Utils::InsensitiveCompare(groupName, cg.Name) || Utils::InsensitiveCompare(groupName, allString)) {
            NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCommands:"); 
            std::string textToSend = "";
            for(auto &cm : Commands) {
                if ((cm.RankShow > playerRank) || (cm.Rank > playerRank) || (cm.Hidden) || (!Utils::InsensitiveCompare(cm.Group, groupName) && !Utils::InsensitiveCompare(groupName, allString)))
                    continue;
                std::string textAdd = cm.Name + " &3| &f";
                if (64 - textToSend.size() >= textAdd.size())
                    textToSend += textAdd;
                else {
                    NetworkFunctions::SystemMessageNetworkSend(c->Id, textToSend);
                    textToSend = textAdd;
                }
            }
            if (textToSend.size() > 0) {
                NetworkFunctions::SystemMessageNetworkSend(c->Id, textToSend);
            }
            found = true;
            break;
        }
    }

    if (!found) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't find command group '" + groupName + "'");
    }
}

void CommandMain::CommandHelp() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);

    bool found = false;
    for(auto cmd : Commands) {
        if (!Utils::InsensitiveCompare(cmd.Name, ParsedOperator[0]))
            continue;

        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCommand Help:");
        NetworkFunctions::SystemMessageNetworkSend(c->Id, cmd.Description);
        found = true;
        break;
    }

    if (!found) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't find command '" + ParsedOperator[0] + "'");
    }
}

void CommandMain::CommandPlayers() {
     Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
     NetworkFunctions::SystemMessageNetworkSend(c->Id, "&ePlayers:");
    std::string textToSend = "";

     for(auto const &nc : nm->_clients) {
         if (nc.second != nullptr && nc.second->player != nullptr && nc.second->player->tEntity->playerList != nullptr) {
             std::string playerName = Entity::GetDisplayname(nc.second->player->tEntity->Id);

            std::string toAdd = playerName + " &c| ";
            if (64 - textToSend.size() >= toAdd.size()) {
                textToSend += toAdd;
            } else {
                NetworkFunctions::SystemMessageNetworkSend(c->Id, textToSend);
                textToSend = toAdd;
            }
         }
     }
     if (textToSend.size() > 0) {
         NetworkFunctions::SystemMessageNetworkSend(c->Id, textToSend);
     }
}

void CommandMain::CommandPlayerInfo() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    Rank* rm = Rank::GetInstance();

    PlayerListEntry* ple = pll->GetPointer(ParsedOperator[0]);
    if (ple == nullptr) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't find a player named '" + ParsedOperator[0] + "'");
        return;
    }
    RankItem ri = rm->GetRank(ple->PRank, false);
    std::string textTosend = "&ePlayer Info: <br>";
    textTosend += "&eNumber: " + stringulate(ple->Number) + "<br>";
    textTosend += "&eName: " + ple->GetDisplayName() + "<br>";
    textTosend += "&eRank: " + ri.Name + " (" + stringulate(ple->PRank) + ")<br>";
    textTosend += "&eIP: " + ple->IP + "<br>";
    textTosend += "&eOntime: " + stringulate(ple->OntimeCounter/3600.0) + "h<br>";
    textTosend += "&eLogins: " + stringulate(ple->LoginCounter) + "<br>";
    textTosend += "&eKicks: " + stringulate(ple->KickCounter) + "<br>";
    
    if (ple->Banned) {
        textTosend += "&4Player is banned<br>";
    }
    if (ple->Stopped) {
        textTosend += "&4Player is stopped<br>";
    }
    if (ple->MuteTime > time(nullptr)) {
        textTosend += "&4Player is muted<br>";
    }
    NetworkFunctions::SystemMessageNetworkSend(c->Id, textTosend);
}

void CommandMain::CommandChangeRank() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    Rank* rm = Rank::GetInstance();

    std::string playerName = ParsedOperator[0];
    int rankVal = std::stoi(ParsedOperator[1]);
    std::string reason = ParsedText2;

    if (rankVal >= -32768 && rankVal <= 32767) {
        PlayerListEntry* ple = pll->GetPointer(ParsedOperator[0]);
        if (ple == nullptr) {
            NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't find a player named '" + ParsedOperator[0] + "'");
            return;
        }
        if (c->player->tEntity->playerList->PRank <= ple->PRank) {
            NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't modify the rank of someone higher than you");
            return;
        }
        if (c->player->tEntity->playerList->PRank <= rankVal) {
            NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't set a rank higher than your own");
            return;
        }
        ple->SetRank(rankVal, reason);
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&ePlayers rank was updated.");
    }
}

void CommandMain::CommandGlobal() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);

    if (c == nullptr)
        return;

    std::string onString = "on";
    std::string trueString = "true";
    std::string offString = "off";
    std::string falseString = "false";

    if (Utils::InsensitiveCompare(ParsedOperator[0], onString) || Utils::InsensitiveCompare(ParsedOperator[0], trueString)) {
        c->GlobalChat = true;
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eGlobal chat is now on by default.");
    } else if (Utils::InsensitiveCompare(ParsedOperator[0], offString) || Utils::InsensitiveCompare(ParsedOperator[0], falseString)) {
        c->GlobalChat = false;
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eGlobal chat is now off by default.");
    } else {
        c->GlobalChat = !c->GlobalChat;
        std::string status = (c->GlobalChat ? "on" : "off");
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eGlobal chat is now " + status + " by default.");
    }
}

void CommandMain::CommandPing() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);

    if (c == nullptr)
        return;

    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eYour ping is " + stringulate(c->Ping) + "ms.");
    MapMain* mm = MapMain::GetInstance();
    mm->AddFillAction(CommandClientId, c->player->MapId, "terrabyte", "");
}

void CommandMain::CommandChangeMap() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);

    if (c == nullptr)
        return;

    MapMain* mm = MapMain::GetInstance();
    std::shared_ptr<Map> mi = mm->GetPointer(ParsedText0);
    if (mi != nullptr) {
        c->player->tEntity->PositionSet(mi->data.ID, mi->data.SpawnX, mi->data.SpawnY, mi->data.SpawnZ, mi->data.SpawnRot, mi->data.SpawnLook, 255, true);

    } else {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eUnable to find map '" + ParsedText0 + "'.");
    }
}

void CommandMain::CommandSaveMap() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);

    if (c == nullptr)
        return;

    MapMain* mm = MapMain::GetInstance();
    Files* fm = Files::GetInstance();
    std::string mapDirectory = "";

    if (ParsedText0 != "") {
        mapDirectory = fm->GetFolder("Maps") + ParsedText0;
        if (mapDirectory.substr(mapDirectory.size()-2, 1) != "/") {
            mapDirectory += "/";
        }
    }

    mm->AddSaveAction(CommandClientId, c->player->MapId, mapDirectory);
    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eSave Queued.");
}

void CommandMain::CommandGetRank() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    Rank* rm = Rank::GetInstance();
    PlayerListEntry* ple;
    
    if (ParsedOperator[0].empty()) {
        ple = c->player->tEntity->playerList;
    } else {
        ple = pll->GetPointer(ParsedOperator[0]);
    }
    
    if (ple == nullptr) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't find a player named '" + ParsedOperator[0] + "'");
        return;
    }

    RankItem ri = rm->GetRank(ple->PRank, false);
    std::string textTosend = "&ePlayer '" + ple->Name + "' is ranked '" + ri.Prefix + ri.Name + ri.Suffix + "'.";

    NetworkFunctions::SystemMessageNetworkSend(c->Id, textTosend);
}

void CommandMain::CommandSetMaterial() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    Block* bm = Block::GetInstance();

    if (ParsedText0.empty()) {
        c->player->tEntity->buildMaterial = -1;
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eMaterial reset.");
        return;
    }

    MapBlock b = bm->GetBlock(ParsedText0);
    if (b.Id == -1) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't find a block called '" + ParsedText0 + "'.");
        return;
    }
    c->player->tEntity->buildMaterial = b.Id;
    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eYour build material is now " + b.Name);
}

void CommandMain::CommandKick() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple;

    ple = pll->GetPointer(ParsedOperator[0]);
    if (ple == nullptr) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't find a player named '" + ParsedOperator[0] + "'");
        return;
    }
    std::string kickReason = (ParsedText1.empty() ? "&eYou were kicked." : ParsedText1);

    if (c->player->tEntity->playerList->PRank > ple->PRank) {
        ple->Kick(kickReason, ple->KickCounter+1, true, true);
    } else {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't kick someone ranked higher than you.");
        return;
    }
}

void CommandMain::CommandBan() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple;

    ple = pll->GetPointer(ParsedOperator[0]);
    if (ple == nullptr) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't find a player named '" + ParsedOperator[0] + "'");
        return;
    }
    std::string banReason = (ParsedText1.empty() ? "&eYou were banned." : ParsedText1);
    if (c->player->tEntity->playerList->PRank > ple->PRank) {
        ple->Ban(banReason);
    } else {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't ban someone ranked higher than you.");
        return;
    }
}

void CommandMain::CommandUnban() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple;

    ple = pll->GetPointer(ParsedOperator[0]);
    if (ple == nullptr) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't find a player named '" + ParsedOperator[0] + "'");
        return;
    }
    if (c->player->tEntity->playerList->PRank > ple->PRank) {
        ple->Unban();
    } else {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't ban someone ranked higher than you.");
        return;
    }
}

void CommandMain::CommandStop() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple;

    ple = pll->GetPointer(ParsedOperator[0]);
    if (ple == nullptr) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't find a player named '" + ParsedOperator[0] + "'");
        return;
    }
    std::string stopReason = (ParsedText1.empty() ? "&eYou were stopped." : ParsedText1);
    if (c->player->tEntity->playerList->PRank > ple->PRank) {
        ple->Stop(stopReason);
    } else {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't stop someone ranked higher than you.");
        return;
    }
}

void CommandMain::CommandUnStop() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple;

    ple = pll->GetPointer(ParsedOperator[0]);
    if (ple == nullptr) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't find a player named '" + ParsedOperator[0] + "'");
        return;
    }
    if (c->player->tEntity->playerList->PRank > ple->PRank) {
        ple->Unstop();
    } else {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't stop someone ranked higher than you.");
        return;
    }
}

void CommandMain::CommandMute() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    PlayerListEntry* ple;

    ple = pll->GetPointer(ParsedOperator[0]);
    if (ple == nullptr) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't find a player named '" + ParsedOperator[0] + "'");
        return;
    }
    if ((ParsedOperator[1].empty())) {
        ParsedOperator[1] = "99999";
    }

    if (c->player->tEntity->playerList->PRank > ple->PRank) {
        ple->Mute(stoi(ParsedOperator[1]), "Muted by " + c->player->LoginName);
    } else {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't mute someone ranked higher than you.");
        return;
    }
}

void CommandMain::CommandUnmute() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    Player_List* pll = Player_List::GetInstance();
    Rank* rm = Rank::GetInstance();
    PlayerListEntry* ple;

    ple = pll->GetPointer(ParsedOperator[0]);
    if (ple == nullptr) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't find a player named '" + ParsedOperator[0] + "'");
        return;
    }
    if (c->player->tEntity->playerList->PRank > ple->PRank) {
        ple->Unmute();
    }else {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't unmute someone ranked higher than you.");
        return;
    }
}

void CommandMain::CommandMaterials() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eMaterials:");
    Block* bm = Block::GetInstance();
    std::string toSend = "";
    for (int i = 0; i < 255; ++i) {
        MapBlock block = bm->GetBlock(i);
        std::string toAdd = "";
        if (block.Special && block.RankPlace <= c->player->tEntity->playerList->PRank) {
            toAdd += "&e" + block.Name + " &f| ";
            if (64 - toSend.size() >= toAdd.size())
                toSend += toAdd;
            else {
                NetworkFunctions::SystemMessageNetworkSend(c->Id, toSend);
                toSend = toAdd;
            }

        }
    }
    if (!toSend.empty()) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, toSend);
    }
}

void CommandMain::CommandListMaps() {
    Network* nm = Network::GetInstance();
    MapMain* mapMain = MapMain::GetInstance();

    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eMaps:");
    std::string toSend = "";
    for(auto const &map : mapMain->_maps) {
        std::string toAdd = "";
        if (map.second->data.RankShow <= c->player->tEntity->playerList->PRank) {
            toAdd += "&e" + map.second->data.Name + " &f| ";
            if (64 - toSend.size() >= toAdd.size())
                toSend += toAdd;
            else {
                NetworkFunctions::SystemMessageNetworkSend(c->Id, toSend);
                toSend = toAdd;
            }

        }
    }
    if (!toSend.empty()) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, toSend);
    }
}

void CommandMain::CommandServerInfo() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    std::string serverRunTime = stringulate(time(nullptr) - System::startTime / 120.0);

    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eServer Info:");
#ifdef __linux__
    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eD3PP v" + stringulate(SYSTEM_VERSION_NUMBER) + ", Linux (x64)");
#else
#ifdef MSVC
    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eD3PP v" + stringulate(SYSTEM_VERSION_NUMBER) + ", Windows [MSVC] (x86)");
#else
    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eD3PP v" + stringulate(SYSTEM_VERSION_NUMBER) + ", Windows (x64)");
#endif
#endif
    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eRun time: " + serverRunTime + "h");
    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eServer Memory Allocations: " + stringulate(Mem::MemoryUsage / 4096) + " MB");
}

void CommandMain::CommandLogLast() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);

    int numLines = 10;

    if (!ParsedOperator[0].empty()) {
        numLines = stoi(ParsedOperator[0]);
    }
    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eLog:");
    Logger* logMain = Logger::GetInstance();
    if (numLines > logMain->Messages.size()) {
        numLines = logMain->Messages.size();
    }
    int logSize = logMain->Messages.size();
    for(int i = 0; i< numLines; i++) {
        LogMessage message = logMain->Messages.at(logSize-i-1);
        NetworkFunctions::SystemMessageNetworkSend(c->Id, " " + message.Message);
    }
}

void CommandMain::CommandUndoTime() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);

    int timeAmount = 60;

    if (!ParsedOperator[0].empty()) {
        timeAmount = stoi(ParsedOperator[0]);
    }
    if (timeAmount > 3600)
        timeAmount = 3600;
    time_t doTime = time(nullptr) - timeAmount;
    Undo::UndoTime(c->player->MapId, doTime);
    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eBlockchanges undone.");
}

void CommandMain::CommandUndoPlayer() {
    Network* nm = Network::GetInstance();
    Player_List* playerList= Player_List::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);
    PlayerListEntry* entry= playerList->GetPointer(ParsedOperator[0]);
    if (entry == nullptr) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eUnable to find a player named '" + ParsedOperator[0] + "'.");
        return;
    }

    int timeAmount = 60;

    if (!ParsedOperator[1].empty()) {
        timeAmount = stoi(ParsedOperator[1]);
    }
    if (timeAmount > 3600)
        timeAmount = 3600;

    time_t doTime = time(nullptr) - timeAmount;
    Undo::UndoPlayer(c->player->MapId, entry->Number, doTime);

    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eBlockchanges undone.");
}

void CommandMain::CommandUndo() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);

    int timeAmount = 60;

    if (!ParsedOperator[0].empty()) {
        timeAmount = stoi(ParsedOperator[0]);
    }
    if (timeAmount > 3600)
        timeAmount = 3600;

    time_t doTime = time(nullptr) - timeAmount;
    Undo::UndoPlayer(c->player->MapId, c->player->tEntity->playerList->Number, doTime);

    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eBlockchanges undone.");
}
