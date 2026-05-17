//
// Created by unknown on 5/9/26.
//

#include "gui/maps.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "common/Logger.h"
#include "world/Map.h"
#include "world/MapMain.h"

Maps::Maps() = default;

Maps::~Maps() = default;

void Maps::Draw() {
    auto* mm = D3PP::world::MapMain::GetInstance();
    static int selected = 0;
    // -- Left Section
    {
        if (ImGui::BeginChild("Left Pane", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX)) {
            for (int i = 0; i < mm->_maps.size(); i++) {
                auto thisMap = mm->_maps[i];
                if (ImGui::Selectable(thisMap->Name().c_str(), selected == i, ImGuiSelectableFlags_SelectOnNav)) {
                    selected = i;
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();
    // -- Right section
    {
        auto selectedMap = mm->_maps[selected];
        ImGui::BeginGroup();
        ImGui::BeginChild("Map View", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
        ImGui::Text("Map %s", selectedMap->Name().c_str());
        ImGui::Separator();
        if (ImGui::BeginTabBar("##MapTabs", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("Details")) {
                auto mapSize = selectedMap->GetSize();
                ImGui::Text("Size: %dx%dx%d", mapSize.X, mapSize.Y, mapSize.Z);
                ImGui::SameLine();
                if (ImGui::Button("Resize")) {
                    // -- Popup..

                }
                // --
                ImGui::Text("Status: %s", selectedMap->loaded ? "Loaded" : "Unloaded");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Permissions")) {

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();
        ImGui::EndGroup();
    }
}
