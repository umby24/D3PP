//
// Created by Wande on 2/23/2021.
//

#include "Rank.h"
#include <iomanip>

#include "Utils.h"
#include "Files.h"
#include "Logger.h"

const std::string MODULE_NAME = "Rank";
Rank* Rank::Instance = nullptr;

Rank::Rank() {
    this->Interval = std::chrono::seconds(2);
    this->Setup = [this] { Load(); };
    this->Main= [this] { MainFunc(); };
    this->Teardown = [this] { Save(); };
    SaveFile = false;
    LastFileDate = 0;

    TaskScheduler::RegisterTask("Rank", *this);
}

void Rank::Save() {
    json j;
    Files* f = Files::GetInstance();
    std::string rankFile = f->GetFile(RANK_FILE_NAME);

    for(const auto& x : _ranks) {
        struct RankItem item = x.second;
        std::string key = stringulate(item.Rank);

        j[key] = nullptr;
        j[key]["Rank"] = item.Rank;
        j[key]["Name"] = item.Name;
        j[key]["Prefix"] = item.Prefix;
        j[key]["Suffix"] = item.Suffix;
        j[key]["OnClient"] = item.OnClient;
    }

    std::ofstream oStream(rankFile, std::ios::trunc);
    oStream << std::setw(4) << j;
    oStream.flush();
    oStream.close();

    time_t modTime = Utils::FileModTime(rankFile);
    LastFileDate = modTime;

    Logger::LogAdd(MODULE_NAME, "File saved.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void Rank::MainFunc() {
    if (SaveFile) {
        Save();
        SaveFile = false;
    }

    Files* f = Files::GetInstance();
    std::string blockFile = f->GetFile(RANK_FILE_NAME);
    time_t modTime = Utils::FileModTime(blockFile);

    if (modTime != LastFileDate) {
        Load();
        LastFileDate = modTime;
    }
}

void Rank::Load() {
    Files* f = Files::GetInstance();
    std::string filePath = f->GetFile(RANK_FILE_NAME);
    json j;

    std::ifstream iStream(filePath);

    if (!iStream.is_open()) {
        Logger::LogAdd(MODULE_NAME, "Failed to load ranks!!", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        DefaultRanks();
        return;
    }

    iStream >> j;
    iStream.close();

    for(auto &item : j) {
        struct RankItem loadedItem;
        loadedItem.Rank = item["Rank"];
        loadedItem.Name = item["Name"];
        loadedItem.Prefix = item["Prefix"];
        loadedItem.Suffix = item["Suffix"];
        loadedItem.OnClient = item["OnClient"];
        _ranks[loadedItem.Rank] = loadedItem;
    }

    Logger::LogAdd("Rank", "File loaded.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__ );

    time_t modTime = Utils::FileModTime(filePath);
    LastFileDate = modTime;

    if (_ranks.empty()) {
        DefaultRanks();
    }
}

void Rank::Add(RankItem item) {
    _ranks[item.Rank] = item;
    Logger::LogAdd(MODULE_NAME, "Rank added [" + stringulate(item.Rank) + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__ );
    SaveFile = true;
}

RankItem Rank::GetRank(const int rank, bool exact) {
    struct RankItem result;
    bool found = false;

    if (exact) {
        if (_ranks.find(rank) != _ranks.end()) {
            found = true;
            result = _ranks[rank];
        }
    } else {
        int currentRank = -32769;
        for(const auto& x : _ranks) {
            if (rank >= x.first && currentRank < x.first) {
                currentRank = x.first;
                found = true;
            }
        }
        result = _ranks[currentRank];
    }

    if (found)
        return result;

    Logger::LogAdd(MODULE_NAME, "Can't find rank [" + stringulate(rank) + "]", NORMAL, __FILE__, __LINE__, __FUNCTION__);
    return result;
}

void Rank::Delete(int id) {
    if (_ranks.find(id) == _ranks.end())
        return;

    _ranks.erase(id);
    SaveFile = true;
}

void Rank::DefaultRanks() {
    struct RankItem greiferRank {-32767, "Griefer", "&d"};
    struct RankItem guestRank {0, "Guest", "&f"};
    struct RankItem builderRank {50, "Builder", "&e"};
    struct RankItem opRank {150, "Op", "&9", "", 100};
    struct RankItem owner {260, "Owner", "&7", "", 100};

    Add(greiferRank);
    Add(guestRank);
    Add(builderRank);
    Add(opRank);
    Add(owner);
}

Rank *Rank::GetInstance() {
    if (Instance == nullptr)
        Instance = new Rank();

    return Instance;
}

