#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <algorithm>
#include <ctime>
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
    std::ostringstream oss;
    oss << std::setprecision(6) << std::fixed;
    oss << v;
    return oss.str();
}

class Utils
{
    public:
        Utils();
        static void replaceAll(std::string &str, const std::string &from, const std::string &to);
        static int FileSize(std::string filePath);
        static long FileModTime(std::string filePath);
        static bool DirectoryExists(std::string filePath, bool create);
        static bool InsensitiveCompare(std::string &first, std::string &second);
        static std::string TrimPathString(std::string input);
        static void padTo(std::string &str, const size_t num, const char paddingChar = ' ');
        static int strCount(std::string input, char search);
        static std::vector<std::string> splitString(std::string input, const char splitChar = ' ');
protected:

    private:
};

#endif // UTILS_H
