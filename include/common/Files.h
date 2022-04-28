#ifndef FILES_H
#define FILES_H

#include <string>
#include <map>

class Files
{
    public:
        virtual ~Files();
        static std::string GetFile(std::string name);
        static std::string GetFolder(std::string name);
        static void Save();
        static void Load();
    private:
        static void LoadDefault();
        static std::map<std::string, std::string> files;
        static std::map<std::string, std::string> folders;
};

#endif // FILES_H
