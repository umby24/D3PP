#include "Logger.h"

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
    strftime(buffer, sizeof(buffer), "%m-%d-%Y_%H:%M:%S", localtime(&last.Time));
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
        std::cout << message.File << std::setw(15) << "| ";
        std::cout << message.Module << ": ";
        std::cout << message.Message << std::endl;
    }
}

void Logger::LogAdd(std::string module, std::string message, LogType type, std::string file, int line, std::string procedure) {
    struct LogMessage newMessage;
    newMessage.Module = module;
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
