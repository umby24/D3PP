//
// Created by Wande on 4/16/2021.
//

#include "common/PreferenceLoader.h"

PreferenceLoader::PreferenceLoader(std::string filename, std::string folder, bool save) {
    Filename = folder + filename;
    CurrentGroup = "";
    SettingsDictionary.insert(std::make_pair("", std::map<std::string, std::string>()));
    Save = save;
}

void PreferenceLoader::LoadFile() {
    if (!std::filesystem::exists(Filename)) {
        std::ofstream oStream(Filename);
        oStream << "";
        oStream.close();
    }

    LoadFile_();
    LastModified = Utils::FileModTime(Filename);
}

void PreferenceLoader::LoadFile_() {
    CurrentGroup = "";
    SettingsDictionary.clear();

    std::ifstream iStream(Filename);
    std::string thisLine;
    while (std::getline(iStream, thisLine)) {
        if (thisLine.empty() || thisLine[0] == ';')
            continue;

        if (thisLine.find(';') != std::string::npos) // -- mid-line comments
            thisLine = thisLine.substr(0, thisLine.find(';'));

        // -- invalid lines
        if ((thisLine.find('=') == std::string::npos) && (thisLine.find('[') == std::string::npos) && (thisLine.find(']') == std::string::npos)) {
            continue;
        }

        Utils::TrimString(thisLine);

        if ((thisLine[0] == '[') && thisLine[thisLine.size()-1] == ']') {
            std::string groupName = thisLine.substr(1, thisLine.size() - 2);
            while (SettingsDictionary.find(groupName) != SettingsDictionary.end()) {
                groupName += "§";
            }
            SettingsDictionary.insert(std::make_pair(groupName, std::map<std::string, std::string>()));
            CurrentGroup = groupName;
            continue;
        }

        if (thisLine.find('=') == std::string::npos) // -- skip invalid/blank lines
            continue;

        std::string key = thisLine.substr(0, thisLine.find('='));
        std::string value = thisLine.substr(thisLine.find('=') + 1, thisLine.size() - (thisLine.find('=') + 1));
        Utils::TrimString(key);
        Utils::TrimString(value);

        if (CurrentGroup == "" && SettingsDictionary.find("") == SettingsDictionary.end()) {
            SettingsDictionary.insert(std::make_pair("", std::map<std::string, std::string>()));
        }

        if (SettingsDictionary[CurrentGroup].find(key) != SettingsDictionary[CurrentGroup].end())
            SettingsDictionary[CurrentGroup][key] = value;
        else
            SettingsDictionary[CurrentGroup].insert(std::make_pair(key, value));
    }
}

void PreferenceLoader::SaveFile() {
    if (!Save)
        return;

    std::ofstream oStream(Filename);
    for (auto const &pair : SettingsDictionary) {
        if (pair.first != "") {
            std::string grpName = pair.first;
            Utils::replaceAll(grpName, "§", "");
            oStream << "[" << grpName << "]" << std::endl;
        }

        for(auto const &subset : pair.second) {
            oStream << subset.first << " = " << subset.second << std::endl;
        }
    }

    LastModified = Utils::FileModTime(Filename);
}

void PreferenceLoader::SelectGroup(std::string group) {
    CurrentGroup = group;
    if (SettingsDictionary.find(group) != SettingsDictionary.end())
        return;

    SettingsDictionary.insert(std::make_pair(group, std::map<std::string, std::string>()));
}

std::string PreferenceLoader::Read(std::string key, std::string def) {
    if (SettingsDictionary[CurrentGroup].find(key) != SettingsDictionary[CurrentGroup].end())
        return SettingsDictionary[CurrentGroup][key];

    SettingsDictionary[CurrentGroup].insert(std::make_pair(key, def));
    return def;
}

int PreferenceLoader::Read(std::string key, int def) {
    if (SettingsDictionary[CurrentGroup].find(key) != SettingsDictionary[CurrentGroup].end())
        return stoi(SettingsDictionary[CurrentGroup][key]);

    SettingsDictionary[CurrentGroup].insert(std::make_pair(key, stringulate(def)));
    return def;
}

void PreferenceLoader::Write(std::string key, std::string value) {
    if (SettingsDictionary[CurrentGroup].find(key) != SettingsDictionary[CurrentGroup].end()) {
        SettingsDictionary[CurrentGroup][key] = value;
        return;
    }

    SettingsDictionary[CurrentGroup].insert(std::make_pair(key, value));
}

void PreferenceLoader::Write(std::string key, int value) {
    if (SettingsDictionary[CurrentGroup].find(key) != SettingsDictionary[CurrentGroup].end()) {
        SettingsDictionary[CurrentGroup][key] = stringulate(value);
        return;
    }

    SettingsDictionary[CurrentGroup].insert(std::make_pair(key, stringulate(value)));
}
