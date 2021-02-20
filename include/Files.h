#ifndef FILES_H
#define FILES_H
#include <iostream>
#include <fstream>
#include <iomanip>
#include <json.hpp>
#include <map>

#include "Utils.h"
#include "Logger.h"

using json = nlohmann::json;
using namespace std;

class Files
{
    public:
        static Files* GetInstance();

        Files();
        virtual ~Files();
        std::string GetFile(std::string name);
        std::string GetFolder(std::string name);
        void Save();
        void Load();
    protected:
        static Files* singleton_;
    private:
        std::map<std::string, std::string> files;
        std::map<std::string, std::string> folders;
};

#endif // FILES_H
