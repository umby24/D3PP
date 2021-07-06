#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <vector>

const int LOG_FILE_SIZE_MAX = 1000000;
const int LOG_SIZE_MAX = 1000;

enum LogType {
    NORMAL = 0,
    CHAT = 1,
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
        static void LogAdd(std::string module, std::string message, LogType type, std::string file, int line, std::string procedure);
    protected:
        void Add(struct LogMessage message);
        static Logger* GetInstance();
        static Logger* singleton_;
    private:
        int Timer;
        bool SaveFile;
        std::string Filename;
        bool GuiOutput;
        std::vector<LogMessage> Messages;
        std::ofstream fileStream;

        void SizeCheck();
        void FileWrite();
};

#endif // LOGGER_H
