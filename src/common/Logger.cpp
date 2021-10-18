#include "common/Logger.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <mutex>

#include "Utils.h"
#include "common/Files.h"

Logger* Logger::singleton_ = nullptr;

Logger::Logger()
{
    GuiOutput = false;
    //ctor
}

Logger::~Logger()
{
    //dtor
    if (fileStream.is_open()) {
        fileStream.flush();
        fileStream.close();
    }
}

Logger* Logger::GetInstance() {
    if (singleton_ == nullptr)
        singleton_ = new Logger();

    return singleton_;
}

void Logger::FileWrite() {
    SizeCheck();

    if (!fileStream.is_open())
        return;

    char buffer[255];
    struct LogMessage last = Messages[Messages.size() - 1];
    std::tm bt {};
#if defined(__unix__)
    localtime_r(&last.Time, &bt);
#elif defined(_MSC_VER)
    localtime_s(&bt, (const time_t*)&last.Time);
#else
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    bt = *std::localtime((const time_t*)(&last.Time));
#endif

    strftime(buffer, sizeof(buffer), "%m-%d-%Y_%H:%M:%S", &bt);
    fileStream << buffer << ">";
    fileStream << last.File << std::setw(15) << " | ";
    fileStream << last.Line << std::setw(4) << " | ";
    fileStream << last.Module << std::setw(15) << ": ";
    fileStream << last.Message << std::endl;
}

void Logger::Add(struct LogMessage message) {
    Messages.push_back(message);
    FileWrite();

    if (Messages.size() > LOG_SIZE_MAX) {
        Messages.erase(Messages.begin());
    }

    if (GuiOutput) {
        std::cout << message.File << "|";
        std::cout << message.Line << "|";
        std::cout << message.Module << "|";
        std::cout << message.Message << std::endl;
    } else {
        if (message.File.size() > 15)
            message.File = message.File.substr(0, 15);
        else
            Utils::padTo(message.File, 15);

        char buffer[255];
        std::tm bt {};
#if defined(__unix__)
        localtime_r(&message.Time, &bt);
#elif defined(_MSC_VER)
        localtime_s(&bt, (const time_t*)&message.Time);
#else
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        bt = *std::localtime((const time_t*)(&message.Time));
#endif

        strftime(buffer, sizeof(buffer), "%H:%M:%S", &bt);
        std::cout << "[" << buffer << "] ";

        switch(message.Type) {
            case LogType::VERBOSE:
                std::cout << "\x1B[30m[Verbose] ";
                std::cout << "\x1B[37m" << message.File << "| ";
                std::cout << "\x1B[33m" << message.Module << ": ";
                break;
            case LogType::DEBUG:
                std::cout << "\x1B[30m[Debug] ";
                std::cout << "\x1B[37m" << message.File << "| ";
                std::cout << message.Module << ": ";
                break;
            case LogType::WARNING:
                std::cout << "\x1B[93m[Warning] ";
                std::cout << "\x1B[37m" << message.File << "| ";
                std::cout << message.Module << ": ";
                break;
            case LogType::L_ERROR:
                std::cout << "\x1B[91m[Error]\x1B[37m ";
                std::cout << message.File << "| ";
                std::cout << message.Module << ": ";
                break;
            case LogType::CHAT:
                std::cout << "\x1B[36m[Chat]\x1B[97m ";
                break;
            case LogType::COMMAND:
                std::cout << "\x1B[32m[Command]\x1B[97m ";
                break;
            case LogType::NORMAL:
                std::cout << "\x1B[92m[Info]\x1B[97m ";
                break;
        }

        std::cout << message.Message << std::endl;
    }
}

void Logger::LogAdd(std::string moadule, std::string message, LogType type, std::string file, int line, std::string procedure) {
    struct LogMessage newMessage;
    newMessage.Module = moadule;
    newMessage.Message = message;
    newMessage.File = Utils::TrimPathString(file);
    newMessage.Line = line;
    newMessage.Procedure = procedure;
    newMessage.Type = type;
    newMessage.Time = time(0);

    Logger* coreLogger = GetInstance();
    coreLogger->Add(newMessage);
}

void Logger::SizeCheck() {
    time_t MaxDate = 2147483647;
    if ((Filename.empty()) || Utils::FileSize(Filename) > LOG_FILE_SIZE_MAX) {
        int number = 0;
        Files* f = Files::GetInstance();

        std::string tempName;
        std::string logFile = f->GetFile("Log");
        time_t fileTime;

        for (auto i = 0; i < 5; i++) {
            tempName = logFile;
            Utils::replaceAll(tempName, "[i]", std::to_string(i));

            fileTime = Utils::FileModTime(tempName);

            if (MaxDate > fileTime) {
                MaxDate = fileTime;
                number = i;
            }
        }

        Filename = logFile;
        Utils::replaceAll(Filename, "[i]", std::to_string(number));
        Utils::replaceAll(Filename, "[date]", std::to_string(fileTime));
        fileStream.open(Filename);
    }
}
