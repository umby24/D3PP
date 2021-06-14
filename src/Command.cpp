#include "Command.h"

const std::string MODULE_NAME = "Command";
CommandMain* CommandMain::Instance = nullptr;

CommandMain::CommandMain() {
    TaskItem thisItem;
    thisItem.Setup = [this] { Setup(); };
    thisItem.Main = [this] { MainFunc(); };
    thisItem.Interval = std::chrono::seconds(1);
    TaskScheduler::RegisterTask("Commands", thisItem);
}

void CommandMain::Setup() {
    Command pingCommand;
    pingCommand.Id = "Ping";
    pingCommand.Name = "ping";
    pingCommand.Internal = true;
    pingCommand.Rank = 0;
    pingCommand.RankShow = 0;
    pingCommand.Function = [this] { CommandMain::CommandPing(); };
    Commands.push_back(pingCommand);

    Command globalCommand;
    globalCommand.Id = "Global";
    globalCommand.Name = "global";
    globalCommand.Internal = true;
    globalCommand.Rank = 0;
    globalCommand.RankShow = 0;
    globalCommand.Function = [this] { CommandMain::CommandGlobal(); };
    Commands.push_back(globalCommand);
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
    Logger::LogAdd(MODULE_NAME, "Parsed command is " + ParsedCommand, LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);

    for (int i = 0; i < COMMAND_OPERATORS_MAX; i++) {
        if (i+1 > splitString.size())
            break;

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

    c->player->tEntity->playerList->SetGlobal(c->GlobalChat);
    // Player_List* pl = Player_List::GetInstance();
    // pl->SaveFile = true;
}

void CommandMain::CommandPing() {
    Network* nm = Network::GetInstance();
    std::shared_ptr<NetworkClient> c = nm->GetClient(CommandClientId);

    if (c == nullptr)
        return;

    NetworkFunctions::SystemMessageNetworkSend(c->Id, "&eYour ping is " + stringulate(c->Ping) + "ms.");
}