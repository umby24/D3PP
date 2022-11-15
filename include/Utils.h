#ifndef UTILS_H
#define UTILS_H
#define GLF __FILE__, __LINE__, __FUNCTION__
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <chrono>

template<typename ValueType>
std::string stringulate(ValueType v)
{
    std::ostringstream oss;
    oss << std::setprecision(6) << std::fixed;
    oss << v;
    return oss.str();
}

template<typename InputIt>
std::string join(InputIt first,
                 InputIt last,
                 const std::string& separator = ", ", const std::string& concluder = "")
{
    if (first == last)
    {
        return concluder;
    }

    std::stringstream ss;
    ss << *first;
    ++first;

    while (first != last)
    {
        ss << separator;
        ss << *first;
        ++first;
    }

    ss << concluder;

    return ss.str();
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
        static bool InsensitiveCompare(std::string first, std::string second);
        static std::string TrimPathString(std::string input);
        static void padTo(std::string &str, size_t num, char paddingChar = ' ');
        static int strCount(const std::string &input, char search);
        static std::vector<std::string> splitString(std::string input,  char splitChar = ' ');
        static void TrimString(std::string &input);
        static int Rgb(int red, int green, int blue);
        static short RedVal(int colorVal);
        static short GreenVal(int colorVal);
        static short BlueVal(int colorVal);
        static bool IsNumeric(std::string input);
        static long long CurrentUnixTime() { return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
protected:

    private:
};

#endif // UTILS_H
