//
// Created by unknown on 5/18/26.
//
#include "gui/playerdb.h"

#include "imgui.h"
#include "common/Player_List.h"

void PlayerDB::Draw() {
    auto pInst = Player_List::GetInstance();
    static int selected = 0;
    // -- Left Section
    {
        if (ImGui::BeginChild("Left Pane", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX)) {
            int i = 0;
            for (auto pdbI : pInst->_pList) {
                if (ImGui::Selectable(pdbI->Name.c_str(), selected == i, ImGuiSelectableFlags_SelectOnNav)) {
                    selected = i;
                }
                i++;
            }
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();
    // -- Right section
    {
        auto selectedPlayer = pInst->_pList[selected];
        ImGui::BeginGroup();
        ImGui::BeginChild("Player View", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
        ImGui::Text("Player %s", selectedPlayer->Name.c_str());
        ImGui::Separator();
        if (ImGui::BeginTabBar("##PlayerTabs", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("Details")) {
                ImGui::Text("Is Banned?: %s", selectedPlayer->Banned ? "Yes" : "No");
                ImGui::Text("Ban Message: %s", selectedPlayer->BanMessage.c_str());
                ImGui::Text("Last IP Address: %s", selectedPlayer->IP.c_str());
                ImGui::Text("Times Kicked: %d", selectedPlayer->KickCounter);
                ImGui::Text("Last Kick Message: %s", selectedPlayer->KickMessage.c_str());
                ImGui::Text("Player Rank: %d", selectedPlayer->PRank);
                ImGui::Text("Ontime: %lf", selectedPlayer->OntimeCounter);
                ImGui::Text("Mute Time: %d", selectedPlayer->MuteTime);
                if (ImGui::Button("Ban")) {
                    selectedPlayer->Ban("Banned from PlayerDB Console");
                }
                if (ImGui::Button("Unban")) {
                    selectedPlayer->Unban();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();
        ImGui::EndGroup();
    }
}
