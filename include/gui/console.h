//
// Created by unknown on 4/20/26.
//

#ifndef D3PP_CONSOLE_H
#define D3PP_CONSOLE_H
#include "imgui.h"
#include "common/Logger.h"

class Console {
public:
    Console();
    ~Console();
    void ClearLog();
    void AddLog(const char* fmt, ...);

    ImVec4 GetColor(LogType type);

    void Draw();
    void ExecCommand(const char* cmd);
private:
    char inputBuffer[1024];
    ImVector<char*> items;
    ImVector<const char*> commands;
    ImVector<char*> history;
    int HistoryPos;
    ImGuiTextFilter filter;
    bool AutoScroll;
    bool ScrollToBottom;
};

#endif //D3PP_CONSOLE_H
