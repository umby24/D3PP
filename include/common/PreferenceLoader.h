//
// Created by Wande on 4/16/2021.
//

#ifndef D3PP_PREFERENCELOADER_H
#define D3PP_PREFERENCELOADER_H
#include <string>
#include <map>
class PreferenceLoader {
public:
    std::string Filename;
    std::string CurrentGroup;
    time_t LastModified;
    std::map<std::string, std::map<std::string, std::string>> SettingsDictionary;
    bool Save;

    explicit PreferenceLoader(const std::string& filename, const std::string& folder = "Settings/", bool save = true);
    void LoadFile();
    void SaveFile();
    void SelectGroup(const std::string& group);
    std::string Read(const std::string& key, std::string def);
    int Read(const std::string& key, int def);
    void Write(const std::string& key, const std::string& value);
    void Write(const std::string& key, int value);
    void Write(const std::string& key, float value);
private:
    void LoadFile_();
};
#endif //D3PP_PREFERENCELOADER_H
