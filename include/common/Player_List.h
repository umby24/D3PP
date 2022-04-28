//
// Created by Wande on 3/21/2021.
//

#ifndef D3PP_PLAYER_LIST_H
#define D3PP_PLAYER_LIST_H
#include <sqlite3.h>
#include <ctime>
#include <string>
#include <memory>
#include <vector>

#include "common/TaskScheduler.h"

#define CREATE_SQL "CREATE TABLE Player_List (Number INTEGER PRIMARY KEY, Name TEXT UNIQUE, Rank INTEGER, Counter_Login INTEGER, Counter_Kick INTEGER, Ontime_Counter FLOAT, IP TEXT, Stopped BOOL, Banned BOOL, Time_Muted INTEGER, Message_Ban TEXT, Message_Kick TEXT, Message_Mute TEXT, Message_Rank TEXT, Message_Stop TEXT, Inventory BLOB, Global INTEGER);"
#define REPLACE_SQL "REPLACE INTO Player_List (Number, Name, Rank, Counter_Login, Counter_Kick, Ontime_Counter, IP, Stopped, Banned, Time_Muted, Message_Ban, Message_Kick, Message_Mute, Message_Rank, Message_Stop, Global) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"
#define PLAYERLIST_FILE_NAME "Playerlist"
#define NUM_PLAYER_ATTRIBUTES 5

class PlayerListEntry {
public:
    short Number;
    std::string Name;
    bool Save;
    int Online;
    double OntimeCounter;
    std::string IP;
    bool Banned;
    std::string BanMessage;
    int LoginCounter;
    int KickCounter;
    std::string KickMessage;
    short PRank;
    std::string RankMessage;
    bool Stopped;
    std::string StopMessage;
    int MuteTime;
    std::string MuteMessage;
    std::string Attributes[5];
    int NumAttributes[5];
    std::string StrAttributes[5];
    bool GlobalChat;
    PlayerListEntry();

    std::string GetDisplayName();
    int GetAttribute(const std::string& attrName);
    std::string GetAttributeStr(const std::string& attrName);

    void SetAttribute(const std::string& attrName, int value);
    void SetAttribute(const std::string& attrName, const std::string& value);

    void SetRank(int rank, const std::string &reason);
    void Kick(const std::string &reason, int count, bool log, bool show);
    void Ban(const std::string& reason);
    void Unban();
    void Mute(int minutes, const std::string& reason);
    void Unmute();
    void Stop(const std::string& reason);
    void Unstop();
    void SetGlobal(bool globalChat);
};

class Player_List : TaskItem {
public:
    Player_List();
    void CreateDatabase();
    void CloseDatabase();
    void OpenDatabase();
    PlayerListEntry* GetPointer(int playerId);
    PlayerListEntry* GetPointer(const std::string& playerName);
    void Load();
    void Save();
    void Add(const std::string& name);
    void MainFunc();
    std::vector<PlayerListEntry> _pList;
    static Player_List* GetInstance();
    bool SaveFile;
protected:
    static Player_List* Instance;
private:
    bool dbOpen;
    std::string fileName;

    sqlite3* db;
    short _numberCounter;

    time_t LastFileTime;

    int GetNumber();
};
#endif //D3PP_PLAYER_LIST_H
