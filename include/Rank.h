//
// Created by Wande on 2/23/2021.
//

#ifndef D3PP_RANK_H
#define D3PP_RANK_H
#include <string>
#include <map>
#include <chrono>

#include "TaskScheduler.h"
#include "json.hpp"

using json = nlohmann::json;
const std::string RANK_FILE_NAME = "Rank";

struct RankItem {
    int Rank;
    std::string Name;
    std::string Prefix;
    std::string Suffix;
    int OnClient;
};

class Rank : TaskItem {
public:
    Rank();
    void Add(RankItem item);
    RankItem GetRank(int rank, bool exact);
    void Delete(int id, bool isExact);
    static Rank* GetInstance();
    std::map<int, RankItem> _ranks;
private:
    static Rank* Instance;
    bool SaveFile;
    time_t LastFileDate;


    void Load();
    void Save();
    void MainFunc();
    void DefaultRanks();
};


#endif //D3PP_RANK_H
