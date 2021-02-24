//
// Created by Wande on 2/23/2021.
//

#ifndef D3PP_RANK_H
#define D3PP_RANK_H
#include <string>
#include <vector>

#include "json.hpp"
#include "Utils.h"
#include "TaskScheduler.h"

using json = nlohmann::json;
const std::string RANK_FILE_NAME = "ranks.json";

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
    RankItem GetRank();
    void Delete(int id);
private:
    bool SaveFile;
    time_t LastFileDate;
    std::vector<RankItem> _ranks;

    void Load();
    void Save();
    void MainFunc();
};


#endif //D3PP_RANK_H
