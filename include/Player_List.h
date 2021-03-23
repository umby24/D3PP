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

#include "Logger.h"
#include "Files.h"
#include "Rank.h"
#include "TaskScheduler.h"
#include "Network.h"

const std::string CREATE_SQL = "CREATE TABLE Player_List (Number INTEGER PRIMARY KEY, Name TEXT UNIQUE, Rank INTEGER, Counter_Login INTEGER, Counter_Kick INTEGER, Ontime_Counter FLOAT, IP TEXT, Stopped BOOL, Banned BOOL, Time_Muted INTEGER, Message_Ban TEXT, Message_Kick TEXT, Message_Mute TEXT, Message_Rank TEXT, Message_Stop TEXT, Inventory BLOB, Global INTEGER);";
const std::string REPLACE_SQL = "REPLACE INTO Player_List (Number, Name, Rank, Counter_Login, Counter_Kick, Ontime_Counter, IP, Stopped, Banned, Time_Muted, Message_Ban, Message_Kick, Message_Mute, Message_Rank, Message_Stop, Global) VALUES ('?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?');";

const std::string PLAYERLIST_FILE_NAME = "Playerlist";
const int NUM_PLAYER_ATTRIBUTES = 5;

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

    int GetAttribute(std::string attrName);
    std::string GetAttributeStr(std::string attrName);

    void SetAttribute(std::string attrName, int value);
    void SetAttribute(std::string attrName, std::string value);

    void SetRank(int rank, const std::string &reason);
    void Kick(std::string reason, int count, bool log, bool show);
    void Ban(std::string reason);
    void Mute(int minutes, std::string reason);
    void Unmute();
    void Stop(std::string reason);
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
    PlayerListEntry* GetPointer(std::string playerName);
    void Load();
    void Save();
    void Add(std::string name);
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
