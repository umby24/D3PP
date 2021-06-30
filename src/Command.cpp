#include "Command.h"

#include "common/PreferenceLoader.h"
#include "Network.h"
#include "Player.h"
#include "Player_List.h"
#include "Entity.h"
#include "Map.h"
#include "Network_Functions.h"
#include "Logger.h"
#include "Files.h"
#include "Utils.h"

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

        ParsedOperator[i] = splitString[i];
    }

    if (input.size() > ParsedCommand.size())
        ParsedText0 = input.substr(ParsedCommand.size()+1);
    else
    {
        ParsedText0 = "";
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

    std::string groupName = ParsedOperator[1];
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
        if (!Utils::InsensitiveCompare(cmd.Name, ParsedOperator[1]))
            continue;

        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCommand Help:");
        NetworkFunctions::SystemMessageNetworkSend(c->Id, cmd.Description);
        found = true;
        break;
    }

    if (!found) {
        NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eCan't find command '" + ParsedOperator[1] + "'");
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