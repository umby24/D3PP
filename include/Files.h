#ifndef FILES_H
#define FILES_H

#include <string>
#include <map>

#include <json.hpp>
using json = nlohmann::json;

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
        void LoadDefault();
        std::map<std::string, std::string> files;
        std::map<std::string, std::string> folders;
};

#endif // FILES_H
