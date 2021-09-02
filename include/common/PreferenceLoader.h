//
// Created by Wande on 4/16/2021.
//

#ifndef D3PP_PREFERENCELOADER_H
#define D3PP_PREFERENCELOADER_H
#include <fstream>
#include <string>
#include <filesystem>
#include <map>

#include "../Utils.h"

class PreferenceLoader {
public:
    std::string Filename;
    std::string CurrentGroup;
    time_t LastModified;
    std::map<std::string, std::map<std::string, std::string>> SettingsDictionary;
    bool Save;

    explicit PreferenceLoader(std::string filename, std::string folder = "Settings/", bool save = true);
    void LoadFile();
    void SaveFile();
    void SelectGroup(std::string group);
    std::string Read(std::string key, std::string def);
    int Read(std::string key, int def);
    void Write(std::string key, std::string value);
    void Write(std::string key, int value);
    void Write(std::string key, float value);
private:
    void LoadFile_();
};
#endif //D3PP_PREFERENCELOADER_H
