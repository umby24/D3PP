//
// Created by Wande on 2/23/2021.
//

#include "Rank.h"
const std::string MODULE_NAME = "Rank";
Rank::Rank() {
    this->Interval = std::chrono::seconds(2);
    this->Setup = [this] { Load(); };
    this->Main= [this] { MainFunc(); };
    this->Teardown = [this] { Save(); };

    TaskScheduler::RegisterTask("Rank", *this);
}

void Rank::Save() {
    json j;
    Files* f = Files::GetInstance();
    std::string rankFile = f->GetFile(RANK_FILE_NAME);

    for(auto i = 0; i < 255; i++) {
        j[i] = nullptr;
        j[i] = {
                {"id", i},
        };
    }

    ofstream oStream(rankFile, std::ios::trunc);
    oStream << std::setw(4) << j;
    oStream.flush();
    oStream.close();

    time_t modTime = Utils::FileModTime(rankFile);
    LastFileDate = modTime;

    Logger::LogAdd(MODULE_NAME, "File saved.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}

void Rank::MainFunc() {

}

void Rank::Load() {
    Files* f = Files::GetInstance();
    std::string filePath = f->GetFile(RANK_FILE_NAME);
    json j;

    ifstream iStream(filePath);

    if (!iStream.is_open()) {
        Logger::LogAdd(MODULE_NAME, "Failed to load ranks!!", LogType::L_ERROR, __FILE__, __LINE__, __FUNCTION__);
        return;
    }

    iStream >> j;
    iStream.close();

    for(auto &item : j) {
        struct RankItem loadedItem;
    }

    Logger::LogAdd("Rank", "File loaded.", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__ );

    time_t modTime = Utils::FileModTime(filePath);
    LastFileDate = modTime;
}

void Rank::Add(RankItem item) {

}

RankItem Rank::GetRank() {
    return RankItem();
}

void Rank::Delete(int id) {

}

