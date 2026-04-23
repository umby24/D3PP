//
// Created by unknown on 4/20/26.
//

#include "../../include/gui/console.h"

#include <cstdio>
#include <string>

#include "Command.h"
#include "ConsoleClient.h"
#include "common/Logger.h"
#include "network/httplib.h"
#include "network/Network_Functions.h"

Console::Console() {
    ClearLog();
    memset(inputBuffer, 0, sizeof(inputBuffer));
    HistoryPos = -1;
    commands.push_back("HELP");
    AutoScroll = true;
    ScrollToBottom = false;
}

Console::~Console() {
    ClearLog();
    for (int i = 0; i < history.size(); i++) {
        ImGui::MemFree(history[i]);
    }
}

void Console::ClearLog() {
    for (int i = 0; i < items.size(); i++) {
        ImGui::MemFree(items[i]);
    }
    items.clear();
}

void Console::AddLog(const char *fmt, ...) {
    // -- I have no idea wtf this does :D
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    buf[sizeof(buf) - 1] = 0;
    va_end(args);
    items.push_back(buf);
}
ImVec4 Console::GetColor(LogType type) {
    switch (type) {
        case LogType::CHAT:
            return ImVec4(1, 1, 1, 1);
        case LogType::DEBUG:
            return ImVec4((99/255.0f), (99/255.0f), (99/255.0f), 1);
        case LogType::NORMAL:
            return ImVec4(0, 1, 0, 1);
        case LogType::VERBOSE:
            return ImVec4((54/255.0f), (54/255.0f), (54/255.0f), 1);
        case LogType::COMMAND:
            return ImVec4((96/255.0f), (96/255.0f), (96/255.0f), 1);
        case LogType::L_ERROR:
            return ImVec4(1.0f, 0.4f, 0.4f, 1);
        case LogType::WARNING:
            return ImVec4((96/255.0f), (96/255.0f), (96/255.0f), 1);
    }
    return ImVec4(96, 96, 96, 1);
}
std::string LogLevelToString(LogType logLevel) {
    switch (logLevel) {
        case NORMAL:
            return "INFO";
        case WARNING:
            return "WARNING";
        case L_ERROR:
            return "ERROR";
        case DEBUG:
            return "DEBUG";
        case VERBOSE:
            return "VERBOSE";
        case CHAT:
            return "CHAT";
        default:
            return "UNKNOWN";
    }
}

void Console::Draw() {
    Logger* lg = Logger::GetInstance();

    ImGuiStyle& style = ImGui::GetStyle();
    const float footer_height_to_reserve = style.SeparatorSize + style.ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar)) {
        if (ImGui::BeginPopupContextWindow()) { // -- Right click options
            if (ImGui::Selectable("Clear")) ClearLog();
            ImGui::EndPopup();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

        for (auto & msg : lg->Messages) {
            ImGui::PushStyleColor(ImGuiCol_Text, GetColor(msg.Type));
            std::string myThing = ("[" + Logger::GetItemTimestamp(msg.Time) + "] {" + LogLevelToString(msg.Type) + "} " + msg.Message);
            ImGui::TextWrapped(myThing.c_str());
            ImGui::PopStyleColor();
        }

        if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
            ImGui::SetScrollHereY(1.0f);

        ScrollToBottom = false;
        ImGui::PopStyleVar();
    }

    ImGui::EndChild();
    ImGui::Separator();
    char someChat[64] = "";

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputText("##ChatInput", someChat, IM_ARRAYSIZE(someChat), ImGuiInputTextFlags_EnterReturnsTrue)) {
        std::string theChat = std::string(someChat);

        if (theChat.substr(0, 1) == "/") {
            const auto cc = ConsoleClient::GetInstance();
            auto cm = CommandMain::GetInstance();
            cm->CommandDo(cc, theChat.substr(1));
            ImGui::SetKeyboardFocusHere(-1);
            return;
        }

        NetworkFunctions::SystemMessageNetworkSend2All(-1, "&c[&fCONSOLE&c]:&f " + std::string(someChat));
        for (int i = 0; i < IM_ARRAYSIZE(someChat); i++) {
            someChat[i] = '\0';
        }

        ImGui::SetKeyboardFocusHere(-1);
    }
}

void Console::ExecCommand(const char *cmd) {
}
