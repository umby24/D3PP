#include <fstream>
#include "Utils.h"

Utils::Utils()
{
    //ctor
}

 void Utils::replaceAll(std::string &str, const std::string &from, const std::string &to) {
    if (from.empty())
        return;

    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

int Utils::FileSize(std::string filePath) {
    struct stat result;

    int statResult = stat(filePath.c_str(), &result);
    if (statResult == 0) return result.st_size;
    return -1;
}

long Utils::FileModTime(std::string filePath) {
    struct stat result;
    int statResult = stat(filePath.c_str(), &result);
    if (statResult == 0) return result.st_mtime;
    return -1;
}

bool Utils::DirectoryExists(std::string filePath, bool create) {
    if (filePath == "")
        return true;

    bool result = std::filesystem::is_directory(filePath);

    if (create && !result) {
        std::filesystem::create_directory(filePath.c_str());
    }
    return result;
}

std::string Utils::TrimPathString(std::string input) {
    if (input.find("/") == std::string::npos && input.find("\\") == std::string::npos)
        return input;

    auto location = input.rfind("/");

    if (location == std::string::npos)
        location = input.rfind("\\");

    return input.substr(location+1);
}

void Utils::padTo(std::string &str, const size_t num, const char paddingChar) {

        if(num > str.size())
            str.insert(0, num - str.size(), paddingChar);

}
