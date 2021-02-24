#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <time.h>
#include <sstream>
#include <filesystem>

#include <sys/types.h>
#include <sys/stat.h>

#ifndef WIN32
#include <unistd.h>
#endif // WIN32

#ifdef WIN32
#define stat _stat
#endif // WIN32

template<typename ValueType>
std::string stringulate(ValueType v)
{
#ifdef __linux__
    return std::to_string(v);
#else
    std::ostringstream oss;
    oss << v;
    return oss.str();
#endif
}

class Utils
{
    public:
        Utils();
        static void replaceAll(std::string &str, const std::string &from, const std::string &to);
        static int FileSize(std::string filePath);
        static long FileModTime(std::string filePath);
        static bool DirectoryExists(std::string filePath, bool create);
        static std::string TrimPathString(std::string input);
    protected:

    private:
};

#endif // UTILS_H
