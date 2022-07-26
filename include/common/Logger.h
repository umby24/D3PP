#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <vector>
#include <mutex>

const int LOG_FILE_SIZE_MAX = 1000000;
const int LOG_SIZE_MAX = 1000;

enum LogType {
    VERBOSE = -3,
    DEBUG,
    NORMAL = 0,
    CHAT = 1,
    COMMAND,
    WARNING = 5,
    L_ERROR = 10
};

struct LogMessage {
    std::string Module;
    std::string Message;
    std::string File;
    int Line;
    std::string Procedure;
    LogType Type;
    time_t Time;
};

class Logger
{
    public:
        Logger();
        virtual ~Logger();
        static void LogAdd(std::string mod, std::string message, LogType type, std::string file, int line, std::string procedure);
        static Logger* GetInstance();
        std::vector<LogMessage> Messages;
    protected:
        void Add(struct LogMessage message);
        static void SubColorCodes(std::string &input);
        static Logger* singleton_;
    private:
        bool SaveFile;
        std::string Filename;
        std::ofstream fileStream;
        std::mutex m_logLock;

        void SizeCheck();
        void FileWrite();
};

#endif // LOGGER_H
