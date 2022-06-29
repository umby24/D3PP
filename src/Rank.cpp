//
// Created by Wande on 2/23/2021.
//

#include "Rank.h"
#include <iomanip>

#include "Utils.h"
#include "common/Files.h"
#include "common/Logger.h"

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
    Files* f = Files::GetInstance();
    std::string rankFile = f->GetFile(RANK_FILE_NAME);


    std::ofstream oStream(rankFile, std::ios::trunc);
    oStream << GetJson();
    oStream.flush();
    oStream.close();

    time_t modTime = Utils::FileModTime(rankFile);
    LastFileDate = modTime;

    Logger::LogAdd(MODULE_NAME, "File saved [" + rankFile + "]", LogType::NORMAL, GLF);
}

void Rank::MainFunc() {
    if (SaveFile) {
        Save();
        SaveFile = false;
    }

    Files* f = Files::GetInstance();
    const std::string blockFile = f->GetFile(RANK_FILE_NAME);
    const time_t modTime = Utils::FileModTime(blockFile);

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
        Logger::LogAdd(MODULE_NAME, "Failed to load ranks!!", LogType::L_ERROR, GLF);
        DefaultRanks();
        return;
    }

    iStream >> j;
    iStream.close();

    SetJson(j);

    Logger::LogAdd("Rank", "File loaded.", LogType::NORMAL, GLF);

    time_t modTime = Utils::FileModTime(filePath);
    LastFileDate = modTime;

    if (m_ranks.empty()) {
        DefaultRanks();
    }
}

void Rank::Add(const RankItem& item) {
    std::scoped_lock<std::mutex> _pqLock(m_rankLock);
    m_ranks[item.Rank] = item;
    Logger::LogAdd(MODULE_NAME, "Rank added [" + stringulate(item.Rank) + "]", LogType::NORMAL, GLF );
    SaveFile = true;
}

RankItem Rank::GetRank(const int rank, bool exact) {
    struct RankItem result;
    bool found = false;

    if (exact) {
        std::scoped_lock<std::mutex> _pqLock(m_rankLock);
        if (m_ranks.find(rank) != m_ranks.end()) {
            found = true;
            result = m_ranks[rank];
        }
    } else {
        int currentRank = -32769;
        std::scoped_lock<std::mutex> _pqLock(m_rankLock);
        for(const auto& x : m_ranks) {
            if (rank >= x.first && currentRank < x.first) {
                currentRank = x.first;
                found = true;
            }
        }
        result = m_ranks[currentRank];
    }

    if (found)
        return result;

    Logger::LogAdd(MODULE_NAME, "Can't find rank [" + stringulate(rank) + "]", NORMAL, GLF);
    return result;
}

void Rank::Delete(int id, bool isExact) {
    RankItem current = GetRank(id, isExact);

    std::scoped_lock<std::mutex> _pqLock(m_rankLock);
    if (m_ranks.find(current.Rank) == m_ranks.end())
        return;

    m_ranks.erase(current.Rank);
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

std::string Rank::GetJson() {
    json j;
    {
        std::scoped_lock<std::mutex> _pqLock(m_rankLock);
        for(const auto& x : m_ranks) {
            struct RankItem item = x.second;
            std::string key = stringulate(item.Rank);

            j[key] = nullptr;
            j[key]["Rank"] = item.Rank;
            j[key]["Name"] = item.Name;
            j[key]["Prefix"] = item.Prefix;
            j[key]["Suffix"] = item.Suffix;
            j[key]["OnClient"] = item.OnClient;
        }
    }


    std::ostringstream oss;
    oss << std::setw(4) << j;
    return oss.str();
}

void Rank::SetJson(json j) {
    std::scoped_lock<std::mutex> _pqLock(m_rankLock);
     for(auto &item : j) {
        struct RankItem loadedItem;
        loadedItem.Rank = item["Rank"];
        loadedItem.Name = item["Name"];
        loadedItem.Prefix = item["Prefix"];
        loadedItem.Suffix = item["Suffix"];
        loadedItem.OnClient = item["OnClient"];
        m_ranks[loadedItem.Rank] = loadedItem;
    }
}
