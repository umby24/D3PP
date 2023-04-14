//
// Created by Wande on 3/22/2021.
//
#include "common/Player_List.h"

#include "network/Network.h"
#include "network/NetworkClient.h"
#include "network/Server.h"
#include "common/Logger.h"
#include "common/Files.h"
#include "Rank.h"
#include "world/Player.h"
#include "network/Network_Functions.h"
#include "world/Entity.h"
#include "common/TaskScheduler.h"
#include "Utils.h"

const std::string MODULE_NAME = "Player_List";
Player_List* Player_List::Instance = nullptr;

PlayerListEntry::PlayerListEntry() : NumAttributes(NUM_PLAYER_ATTRIBUTES) {
    MuteTime = 0;
    Stopped = 0;
    Banned = false;
    Save = false;
    Number = 0;
    OntimeCounter = 0;
    Online = 0;
    LoginCounter = 0;
    KickCounter = 0;
    PRank = 0;
    GlobalChat = false;
}

void Player_List::CloseDatabase() {
    if (dbOpen) {
        dbOpen = false;
        sqlite3_close(db);
        Logger::LogAdd(MODULE_NAME, "Database closed [" + fileName + "]", LogType::DEBUG, __FILE__, __LINE__, __FUNCTION__);
    }
}

static int callback(void* NotUsed, int argc, char **argv, char **azColName) {
    return 0;
}

void Player_List::CreateDatabase() {
    char *zErrMsg = 0;

    sqlite3* tempdb;
    sqlite3_open(fileName.c_str(), &tempdb);
    int rc = sqlite3_exec(tempdb, CREATE_SQL.c_str(), callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        Logger::LogAdd(MODULE_NAME, "Failed to create new DB", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        // -- err :<
        sqlite3_free(tempdb);
        return;
    }

    Logger::LogAdd(MODULE_NAME, "Database created.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    sqlite3_close(tempdb);
}

void Player_List::OpenDatabase() {
    CloseDatabase();
    if (Utils::FileSize(fileName) == -1) {
        CreateDatabase();
    }
    sqlite3_open(fileName.c_str(), &db);
    dbOpen = true;
    Logger::LogAdd(MODULE_NAME, "Database Opened.", LogType::DEBUG, __FILE__, __LINE__, __FUNCTION__);
}

Player_List::Player_List() {
    dbOpen = false;
    fileName = Files::GetFile(PLAYERLIST_FILE_NAME);

    if (fileName.empty())
        fileName = "playerdb.sqlite3";

    _numberCounter = -1;
    this->Setup = [this] { Load(); };
    this->Main = [this] { MainFunc(); };
    this->Teardown = [this] { Save(); };
    this->Interval = std::chrono::minutes(2);
    SaveFile = false;
    db = nullptr;
    LastFileTime = 0;
    TaskScheduler::RegisterTask(MODULE_NAME, *this);
}

void Player_List::Load() {
    OpenDatabase();
    sqlite3_stmt *stmt;
    std::string sqlStatement = "SELECT * FROM Player_List";
    char* errMsg = 0;
    int rc;
    rc = sqlite3_prepare_v2(db, sqlStatement.c_str(), sqlStatement.size(), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        Logger::LogAdd(MODULE_NAME, "Failed to load DB!", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        Logger::LogAdd(MODULE_NAME, "DB Error: " + stringulate(errMsg), LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto newEntry = std::make_shared<PlayerListEntry>();

        sqlite3_column_text(stmt, 3);
        newEntry->Number = sqlite3_column_int(stmt, 0);
        const char *tmp;
        tmp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        newEntry->Name = std::string(tmp);

        newEntry->PRank = sqlite3_column_int(stmt, 2);
        newEntry->LoginCounter = sqlite3_column_int(stmt, 3);
        newEntry->KickCounter = sqlite3_column_int(stmt, 4);
        newEntry->OntimeCounter = sqlite3_column_int(stmt, 5);
        tmp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        if (tmp != nullptr)
        newEntry->IP = std::string(tmp);
        newEntry->Stopped = sqlite3_column_int(stmt, 7);
        newEntry->Banned = sqlite3_column_int(stmt, 8);
        newEntry->MuteTime = sqlite3_column_int(stmt, 9);

        tmp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        if (tmp != nullptr)
        newEntry->BanMessage = std::string(tmp);

        tmp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        if (tmp != nullptr)
        newEntry->KickMessage = std::string(tmp);

        tmp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
        if (tmp != nullptr)
        newEntry->MuteMessage = std::string(tmp);

        tmp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        if (tmp != nullptr)
        newEntry->RankMessage = std::string(tmp);

        tmp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14));
        if (tmp != nullptr)
        newEntry->StopMessage = std::string(tmp);

        newEntry->GlobalChat = sqlite3_column_int(stmt, 16);
        _pList.push_back(newEntry);
    }

    sqlite3_finalize(stmt);
    Logger::LogAdd(MODULE_NAME, "Database Loaded.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    CloseDatabase();

    for(auto const &i : _pList) {
        if ((_numberCounter & 65535) <= (i->Number & 65535))
            _numberCounter = i->Number + 1;
    }
}

void Player_List::Save() {
    OpenDatabase();
    
    for(auto const &i : _pList) {
        if (!i->Save)
            continue;

        sqlite3_stmt *res;
        int rc = sqlite3_prepare_v2(db, REPLACE_SQL.c_str(), -1, &res, nullptr);
        if (rc == SQLITE_OK) {
            sqlite3_bind_int(res, 1, i->Number);
            sqlite3_bind_text(res, 2, i->Name.c_str(), i->Name.size(), nullptr);
            sqlite3_bind_int(res, 3, i->PRank);
            sqlite3_bind_int(res, 4, i->LoginCounter);
            sqlite3_bind_int(res, 5, i->KickCounter);
            sqlite3_bind_int(res, 6, i->OntimeCounter);
            sqlite3_bind_text(res, 7, i->IP.c_str(), i->IP.size(), nullptr);
            sqlite3_bind_int(res, 8, i->Stopped);
            sqlite3_bind_int(res, 9, i->Banned);
            sqlite3_bind_int(res, 10, i->MuteTime);
            sqlite3_bind_text(res, 11, i->BanMessage.c_str(), i->BanMessage.size(), nullptr);
            sqlite3_bind_text(res, 12, i->KickMessage.c_str(), i->KickMessage.size(), nullptr);
            sqlite3_bind_text(res, 13, i->MuteMessage.c_str(), i->MuteMessage.size(), nullptr);
            sqlite3_bind_text(res, 14, i->RankMessage.c_str(), i->RankMessage.size(), nullptr);
            sqlite3_bind_text(res, 15, i->StopMessage.c_str(), i->StopMessage.size(), nullptr);
            sqlite3_bind_int(res, 16, i->GlobalChat);
        }
        int dbSaveResult = sqlite3_step(res);
        sqlite3_finalize(res);

        if (dbSaveResult != SQLITE_DONE)
            Logger::LogAdd(MODULE_NAME, "Error saving PlayerDB.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    }
    Logger::LogAdd(MODULE_NAME, "Database Saved.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    CloseDatabase();
}

std::shared_ptr<PlayerListEntry> Player_List::GetPointer(int playerId) {
    for (auto & i : _pList) {
        if (i->Number == playerId) {
            return i;
        }
    }

    return nullptr;
}

std::shared_ptr<PlayerListEntry> Player_List::GetPointer(std::string player) {
    for (auto & i : _pList) {
        if (i->Name == player) {
            return i;
        }
    }

    return nullptr;
}

void Player_List::MainFunc() {
    if (SaveFile) {
        SaveFile = false;
        Save();
    }
}

void Player_List::Add(std::string name) {
    if (GetPointer(name) != nullptr)
        return;

    auto newEntry = std::make_shared<PlayerListEntry>();
    newEntry->Number = GetNumber();
    newEntry->Name = name;
    newEntry->PRank = 0;
    newEntry->MuteTime = 0;
    newEntry->Stopped = 0;
    newEntry->Save = true;
    newEntry->GlobalChat = true;
    SaveFile = true;
    _pList.push_back(newEntry);
}

int Player_List::GetNumber() {
    int result = _numberCounter;

    while (result == -1 || GetPointer(result) != nullptr) {
        result = _numberCounter;
        _numberCounter++;
    }

    return result;
}

Player_List *Player_List::GetInstance() {
    if (Instance == nullptr)
        Instance = new Player_List();

    return Instance;
}

int PlayerListEntry::GetAttribute(std::string attrName) {
    int result = 0;

    for (auto i = 0; i < NUM_PLAYER_ATTRIBUTES - 1; i++) {
        if (Attributes[i] == attrName) {
            result = NumAttributes[i];
            break;
        }
    }

    return result;
}

std::string PlayerListEntry::GetAttributeStr(std::string attrName) {
    std::string result = "";

    for (auto i = 0; i < NUM_PLAYER_ATTRIBUTES - 1; i++) {
        if (Attributes[i] == attrName) {
            result = StrAttributes[i];
            break;
        }
    }

    return result;
}

void PlayerListEntry::SetAttribute(std::string attrName, int value) {
    bool found = false;
    for (auto i = 0; i < NUM_PLAYER_ATTRIBUTES - 1; i++) {
        if (Attributes[i] == attrName) {
            if (value == 0)
                Attributes[i] = "";

            NumAttributes[i] = value;
            found = true;
            break;
        }
    }

    if (!found) {
        for (auto i = 0; i < NUM_PLAYER_ATTRIBUTES - 1; i++) {
            if (Attributes[i].empty()) {
                if (value != 0)
                    Attributes[i] = attrName;
                NumAttributes[i] = value;
                break;
            }
        }
    }
}

void PlayerListEntry::SetAttribute(std::string attrName, std::string value) {
    bool found = false;
    for (auto i = 0; i < NUM_PLAYER_ATTRIBUTES - 1; i++) {
        if (Attributes[i] == attrName) {
            if (value.empty())
                Attributes[i] = "";

            StrAttributes[i] = value;
            found = true;
            break;
        }
    }

    if (!found) {
        for (auto i = 0; i < NUM_PLAYER_ATTRIBUTES - 1; i++) {
            if (Attributes[i].empty()) {
                if (!value.empty())
                    Attributes[i] = attrName;
                StrAttributes[i] = value;
                break;
            }
        }
    }
}

void PlayerListEntry::SetRank(int rank, const std::string &reason) {
    this->PRank = rank;
    RankMessage = reason;
    Save = true;
    Player_List* i = Player_List::GetInstance();
    i->SaveFile = true;

    Rank* r = Rank::GetInstance();
    std::shared_lock lock(D3PP::network::Server::roMutex, std::defer_lock);
    for(auto &nc : D3PP::network::Server::roClients) {
        if (nc->GetPlayerInstance() && nc->GetPlayerInstance()->GetEntity() && nc->GetPlayerInstance()->GetEntity()->playerList && nc->GetPlayerInstance()->GetEntity()->playerList->Number == Number) {
            RankItem ri = r->GetRank(rank, false);
            Entity::SetDisplayName(nc->GetPlayerInstance()->GetEntity()->Id, ri.Prefix, this->Name, ri.Suffix);
            NetworkFunctions::SystemMessageNetworkSend(nc->GetId(), "&eYour rank has been changed to '" + ri.Name + "' (" + reason + ")");
        }
    }
}

void PlayerListEntry::Kick(const std::string &reason, int count, bool log, bool show) {
    bool found = false;
    {
        std::shared_lock lock(D3PP::network::Server::roMutex, std::defer_lock);
        for (auto &nc: D3PP::network::Server::roClients) {
            if (nc->GetPlayerInstance() && nc->GetPlayerInstance()->GetEntity() &&
                nc->GetPlayerInstance()->GetEntity()->playerList &&
                nc->GetPlayerInstance()->GetEntity()->playerList->Number == Number) {
                nc->Kick("You got kicked (" + reason + ")", !show);
                found = true;
            }
        }
    }

    if (found) {
        KickCounter++;
        KickMessage = reason;
        Save = true;
        Player_List *i = Player_List::GetInstance();
        i->SaveFile = true;
        if (show) {
            NetworkFunctions::SystemMessageNetworkSend2All(-1, "Player " + Name + " was kicked (" + reason + ")");
        }
        if (log) {
            Logger::LogAdd(MODULE_NAME, "Player " + Name + " was kicked (" + reason + ")", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
        }
    }
}

void PlayerListEntry::SetGlobal(bool globalChat) {
    GlobalChat = globalChat;
    Save = true;
    Player_List *i = Player_List::GetInstance();
    i->SaveFile = true;
}

void PlayerListEntry::Mute(int minutes, std::string reason) {
    if (minutes <= 0) {
        minutes = 999999;
    }
    MuteTime = time(nullptr) + (minutes * 60);
    MuteMessage = reason;
    Save = true;
    Player_List *i = Player_List::GetInstance();
    i->SaveFile = true;
    std::string playerName = GetDisplayName();
    NetworkFunctions::SystemMessageNetworkSend2All(-1, "&cPlayer '" + playerName + "'&c was muted. (" + reason + ")");
    Logger::LogAdd(MODULE_NAME, "Player muted: " + Name + " [" + reason + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void PlayerListEntry::Unmute() {
    MuteTime = time(nullptr) - 10;
    Save = true;
    Player_List *i = Player_List::GetInstance();
    i->SaveFile = true;
    std::string playerName = GetDisplayName();
    NetworkFunctions::SystemMessageNetworkSend2All(-1, "&cPlayer '" + playerName + "'&c was unmuted.");
    Logger::LogAdd(MODULE_NAME, "Player unmuted: " + Name, LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void PlayerListEntry::Stop(std::string reason) {
    Stopped = true;
    StopMessage = reason;
    Save = true;
    Player_List *i = Player_List::GetInstance();
    i->SaveFile = true;
    std::string playerName = GetDisplayName();
    NetworkFunctions::SystemMessageNetworkSend2All(-1, "&cPlayer '" + playerName + "'&c was stopped. (" + reason + ")");
    Logger::LogAdd(MODULE_NAME, "Player stopped: " + Name + " [" + reason + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void PlayerListEntry::Unstop() {
    Stopped = false;
    Save = true;
    Player_List *i = Player_List::GetInstance();
    i->SaveFile = true;
    std::string playerName = GetDisplayName();
    NetworkFunctions::SystemMessageNetworkSend2All(-1, "&cPlayer '" + playerName + "'&c was unstopped.");
    Logger::LogAdd(MODULE_NAME, "Player unstopped: " + Name, LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void PlayerListEntry::Ban(std::string reason) {
    Banned = true;
    BanMessage = reason;
    Save = true;
    Player_List *i = Player_List::GetInstance();
    i->SaveFile = true;
    std::string playerName = GetDisplayName();
    NetworkFunctions::SystemMessageNetworkSend2All(-1, "&cPlayer '" + playerName + "'&c was banned. (" + reason + ")");
    Logger::LogAdd(MODULE_NAME, "Player banned: " + Name + " [" + reason + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
    Kick(reason, 1, true, false);
}

std::string PlayerListEntry::GetDisplayName() {
    Rank* rm = Rank::GetInstance();
    RankItem ri = rm->GetRank(PRank, false);
    return ri.Prefix + Name + ri.Suffix;
}

void PlayerListEntry::Unban() {
    Banned = false;
    Save = true;
    Player_List *i = Player_List::GetInstance();
    i->SaveFile = true;
    std::string playerName = GetDisplayName();
    NetworkFunctions::SystemMessageNetworkSend2All(-1, "&cPlayer '" + playerName + "'&c was unbanned.");
    Logger::LogAdd(MODULE_NAME, "Player unbanned: " + Name, LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}
