//
// Created by unknown on 4/2/21.
//

#include "Map.h"

const std::string MODULE_NAME = "Map";

MapMain::MapMain() {
    this->Main = [this] { MainFunc(); };
    this->Interval = std::chrono::seconds(1);
    TaskScheduler::RegisterTask(MODULE_NAME, *this);
}

void MapMain::MainFunc() {
    Files* f = Files::GetInstance();
    std::string mapListFile = f->GetFile(MAP_LIST_FILE);
    long fileTime = Utils::FileModTime(mapListFile);
    if (LastWriteTime != fileTime) {
        // -- MapListLoad
    }
    if (SaveFile && SaveFileTimer < time(nullptr)) {
        SaveFileTimer = time(nullptr) + 5000;
        SaveFile = false;
        // -- MapListSave
    }
    for(auto const &m : _maps) {

        if (m.second->data.SaveInterval > 0 && m.second->data.SaveTime + m.second->data.SaveInterval*60000 < time(nullptr) && m.second->data.loaded) {
            m.second->data.SaveTime = time(nullptr);
            // -- MapActionAddSave
        }
        if (m.second->data.Clients > 0) {
            m.second->data.LastClient = time(nullptr);
            if (!m.second->data.loaded) {
                // -- reload
            }
        }
        if (m.second->data.loaded && (time(nullptr) - m.second->data.LastClient) > 20000) {
            // -- mapunload
        }
    }
    fileTime = Utils::FileModTime(f->GetFile(MAP_SETTINGS_FILE));
    if (fileTime != mapSettingsLastWriteTime) {
        // -- mapSettingsLoad
    }
    if (StatsTimer < time(nullptr)) {
        StatsTimer = time(nullptr) + 5000;
        // -- mapHtmlStats()
    }
}

MapMain* MapMain::GetInstance() {
    if (Instance == nullptr)
        Instance = new MapMain();

    return Instance;
}

shared_ptr<Map> MapMain::GetPointer(int id) {
    if (_maps.find(id) != _maps.end())
        return _maps[id];

    return nullptr;
}

int MapMain::GetMapId() {
    int result = 0;

    while (true) {
        if (GetPointer(result) != nullptr) {
            result++;
            continue;
        }

        return result;
    }
}

std::string MapMain::GetUniqueId() {
    std::string result;

    for(auto i = 1; i < 16; i++)
        result += char(65 + Utils::RandomNumber(25));

    return result;
}

void MapMain::MapListSave() {
    Files* f = Files::GetInstance();
    std::string fName = f->GetFile(MAP_LIST_FILE);
    ofstream oStream(fName);

    for(auto const &m : _maps) {
        oStream << "[" << m.first << "]" << endl;
        oStream << "Name = " << m.second->data.Name << endl;
        oStream << "Directory = " << m.second->data.Directory << endl;
        oStream << "Delete = 0" << endl;
        oStream << "Reload = 0" << endl;
    }
    oStream.close();
    LastWriteTime = Utils::FileModTime(fName);
    Logger::LogAdd(MODULE_NAME, "File saved. [" + fName + "]", LogType::NORMAL, __FILE__, __LINE__, __FUNCTION__);
}
