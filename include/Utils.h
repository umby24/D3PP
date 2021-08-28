#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <iomanip>
#include <sstream>

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
        static int RandomNumber(int max);
        static void replaceAll(std::string &str, const std::string &from, const std::string &to);
        static int FileSize(const std::string &filePath);
        static long FileModTime(const std::string &filePath);
        static bool DirectoryExists(const std::string& filePath, bool create);
        static bool InsensitiveCompare(std::string &first, std::string &second);
        static std::string TrimPathString(std::string input);
        static void padTo(std::string &str, size_t num, char paddingChar = ' ');
        static int strCount(const std::string &input, char search);
        static std::vector<std::string> splitString(std::string input,  char splitChar = ' ');
        static void TrimString(std::string &input);
        static int Rgb(int red, int green, int blue);
        static short RedVal(int colorVal);
        static short GreenVal(int colorVal);
        static short BlueVal(int colorVal);
protected:

    private:
};

#endif // UTILS_H
