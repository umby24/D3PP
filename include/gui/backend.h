//
// Created by unknown on 3/30/26.
//

#ifndef D3PP_BACKEND_H
#define D3PP_BACKEND_H
#include <string>

#include "imgui.h"

class backend {
public:
    int init(std::string resDir = "");
    void beginFrame();
    void render(bool vsync = true);
    void getMouseScreenPos(double& x, double& y);
    void setMouseScreenPos(double x, double y);

    void InputTextCallback(ImGuiInputTextCallbackData *data);

    void ImGuiLoop();

    int renderLoop();
    int end();
private:
    int HistoryPos;
    bool AutoScroll;
    bool ScrollToBottom;
    int SelectedItem;
};

#endif //D3PP_BACKEND_H