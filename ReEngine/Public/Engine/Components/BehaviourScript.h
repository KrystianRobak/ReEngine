#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <string>
#include <vector>

std::vector<std::string> SSplitString(const std::string& str, char delimiter);

struct BehaviourScript
{
    std::string ScriptPath;

    void GenerateGUIElements(std::int32_t entity)
    {

        ImGui::Text("Script File:");


        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.7f, 0.2f, 1.0f));
        ImGui::InputText("##ScriptPathDisplay", ScriptPath.data(), ScriptPath.size() + 1, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor(2);

        // Handle drag-and-drop of a .cpp file
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DIRECTORY_ENTRY"))
            {
                if (payload->DataSize > 0)
                {
                    std::string nameWithPath(static_cast<const char*>(payload->Data));
                    std::vector<std::string> parts = SSplitString(nameWithPath, '|');

                    if (parts.size() == 2)
                    {
                        const std::string& name = parts[0];
                        const std::string& path = parts[1];

                        if (path.ends_with(".cpp"))
                        {
                            ScriptPath = path;
                        }
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
    }
};

