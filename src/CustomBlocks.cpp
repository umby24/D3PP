//
// Created by unknown on 10/29/21.
//

#include "CustomBlocks.h"
#include "common/Files.h"
#include "common/Logger.h"

#include "Utils.h"
#include "json.hpp"
#include <fstream>

using json = nlohmann::json;

CustomBlocks* CustomBlocks::instance = nullptr;

void CustomBlocks::Load() {
    Files* f = Files::GetInstance();
    std::string filePath = f->GetFile(CUSTOM_BLOCK_FILE_NAME);

    if (Utils::FileSize(filePath) == -1) {
        Save();
    }

    json j;
    std::ifstream inFile (filePath);

    if (!inFile.is_open()) {
        Logger::LogAdd("CustomBlocks", "Error loading custom blocks: Can't open blocks file!", LogType::L_ERROR, GLF);
        return;
    }

    try {
        inFile >> j;
    } catch (std::exception e) {
        Logger::LogAdd("CustomBlocks", "Error loading custom blocks: Can't parse file!", LogType::L_ERROR, GLF);
        Logger::LogAdd("CustomBlocks", e.what(), LogType::DEBUG, GLF);
        inFile.close();
        return;
    }
    inFile.close();
    for(auto &item : j) {
        struct BlockDefinition loadedItem { };
        loadedItem.blockId = item["id"];
        loadedItem.name = item["name"];
        loadedItem.solidity = static_cast<BlockSolidity>(item["solidity"]);
        loadedItem.movementSpeed = static_cast<char>(item["movespeed"].get<int>());
        loadedItem.topTexture = static_cast<char>(item["topTexture"].get<int>());
        loadedItem.sideTexture = static_cast<char>(item["sideTexture"].get<int>());
        loadedItem.bottomTexture = static_cast<char>(item["bottomTexture"].get<int>());
        loadedItem.transmitsLight = item["transmitsLight"];
        loadedItem.walkSound = static_cast<char>(item["walkSound"].get<int>());
        loadedItem.fullBright = item["fullBright"];
        loadedItem.shape = static_cast<char>(item["shape"].get<int>());
        loadedItem.drawType = static_cast<char>(item["drawType"].get<int>());
        loadedItem.fogDensity = static_cast<char>(item["fogDensity"].get<int>());
        loadedItem.fogR = static_cast<char>(item["fogR"].get<int>());
        loadedItem.fogR = static_cast<char>(item["fogG"].get<int>());
        loadedItem.fogR = static_cast<char>(item["fogB"].get<int>());
        Add(loadedItem);
    }
    isModified = false;
    Logger::LogAdd("CustomBlocks", "Loaded " + stringulate(_blockDefintiions.size()) + " custom blocks.", LogType::NORMAL, GLF);
}

void CustomBlocks::Add(BlockDefinition blockDef) {
    isModified = true;
    if (_blockDefintiions.contains(blockDef.blockId)) {
        _blockDefintiions[blockDef.blockId] = blockDef;
        return;
    }

    _blockDefintiions.insert(std::make_pair(blockDef.blockId, blockDef));
}

void CustomBlocks::Save() {
    Files* f = Files::GetInstance();
    std::string filePath = f->GetFile(CUSTOM_BLOCK_FILE_NAME);
    json j;
    int index = 0;
    for (auto const &pair : _blockDefintiions) {
        j[index] = nullptr;
        j[index]["id"] = pair.second.blockId;
        j[index]["name"] = pair.second.name;
        j[index]["solidity"] = pair.second.solidity;
        j[index]["movespeed"] = pair.second.movementSpeed;
        j[index]["topTexture"] = pair.second.topTexture;
        j[index]["sideTexture"] = pair.second.sideTexture;
        j[index]["bottomTexture"] = pair.second.bottomTexture;
        j[index]["transmitsLight"] = pair.second.transmitsLight;
        j[index]["walkSound"] = pair.second.walkSound;
        j[index]["fullBright"] = pair.second.fullBright;
        j[index]["shape"] = pair.second.shape;
        j[index]["drawType"] = pair.second.drawType;
        j[index]["fogDensity"] = pair.second.fogDensity;
        j[index]["fogR"] = pair.second.fogR;
        j[index]["fogG"] = pair.second.fogR;
        j[index]["fogB"] = pair.second.fogR;
        index++;
    }


    std::ofstream oStream(filePath, std::ios::trunc);
    oStream << std::setw(4) << j;
    oStream.flush();
    oStream.close();
    Logger::LogAdd("CustomBlocks", "Custom blocks file saved.", LogType::NORMAL, GLF);
}

CustomBlocks::CustomBlocks() : _blockDefintiions() {
    this->Setup = [this]{ Load(); };
    this->Teardown = [this] { Save(); };
    this->Main = [this] { MainFunc(); };
    this->Interval = std::chrono::seconds(1);
    TaskScheduler::RegisterTask("CustomBlocks", *this);
}

void CustomBlocks::MainFunc() {
    if (isModified) {
        Save();
        isModified = false;
    }

    Files* f = Files::GetInstance();
    std::string filePath = f->GetFile(CUSTOM_BLOCK_FILE_NAME);
    time_t currentModTime = Utils::FileModTime(filePath);

    if (currentModTime != lastModified) {
        _blockDefintiions.clear();
        Load();
        lastModified = currentModTime;
    }
}

CustomBlocks *CustomBlocks::GetInstance() {
    if (instance == nullptr)
        instance = new CustomBlocks();

    return instance;
}

std::vector<BlockDefinition> CustomBlocks::GetBlocks() {
    std::vector<BlockDefinition> result;

    for(auto const& pair : _blockDefintiions) {
        result.push_back(pair.second);
    }

    return result;
}

void CustomBlocks::Remove(unsigned char blockId) {
    if (_blockDefintiions.contains(blockId)) {
        isModified = true;
        _blockDefintiions.erase(blockId);
        return;
    }
}
