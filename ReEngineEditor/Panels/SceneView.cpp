#include <Gl/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "SceneView.h"
#include "StaticMesh.h"

inline std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void SceneView::resize(int32_t width, int32_t height)
{
    size.x = width;
    size.y = height;
}

void SceneView::Render()
{
    ImGui::Begin("Scene");

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    size = { viewportPanelSize.x, viewportPanelSize.y };

    ImGui::Image(reinterpret_cast<void*>(viewport->GetTexture()), ImVec2{ size.x, size.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

    // --- Drag and Drop Target: Capture path and trigger pop-up ---
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DIRECTORY_ENTRY")) {
            if (payload->DataSize > 0) {
                std::string nameWithPath(static_cast<const char*>(payload->Data));
                std::vector<std::string> parts = splitString(nameWithPath, '|');

                if (parts.size() == 2) {
                    // 1. Store the data temporarily
                    pendingImportName = parts[0];
                    pendingImportPath = parts[1];

                    // 2. Set the flag to show the pop-up on the next frame
                    showImportTypePopup = true;
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    // --- Pop-up Logic ---
    if (showImportTypePopup) {
        ImGui::OpenPopup("Select Import Type");
        // Keep the flag true until the user makes a choice
    }

    if (ImGui::BeginPopupModal("Select Import Type", &showImportTypePopup, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("File: %s", pendingImportName.c_str());
        ImGui::Separator();
        ImGui::Text("Select the type of mesh to import:");

        // Static Mesh Button
        if (ImGui::Button("Static Mesh", ImVec2(120, 0))) {

            Entity entity = engineAPI->CreateEntity();
            engineAPI->AddComponent(entity, "Transform");

            // Load using loadFBX (Static)
            auto future = engineAPI->GetAssetManager()->loadFBX(pendingImportPath);
            engineAPI->GetAssetManager()->AddPendingMesh(entity, std::move(future));

            ImGui::CloseCurrentPopup();
            showImportTypePopup = false;
        }

        ImGui::SameLine();

        // Skeletal Mesh Button
        if (ImGui::Button("Skeletal Mesh", ImVec2(120, 0))) {

            Entity entity = engineAPI->CreateEntity();
            engineAPI->AddComponent(entity, "Transform");

            // Add Skeletal Mesh Component instead of Static Mesh Component
            engineAPI->AddComponent(entity, "SkeletalMeshComponent");

            // Load using loadSkeletalFBX (Skeletal)
            auto future = engineAPI->GetAssetManager()->loadSkeletalFBX(pendingImportPath);
            engineAPI->GetAssetManager()->AddPendingMesh(entity, std::move(future)); // Still use AddPendingMesh

            ImGui::CloseCurrentPopup();
            showImportTypePopup = false;
        }

        // Optional: Cancel Button
        if (ImGui::Button("Cancel", ImVec2(100, 0))) {
            ImGui::CloseCurrentPopup();
            showImportTypePopup = false;
        }

        ImGui::EndPopup();
    }

    // ---------- Overlay Gizmo Buttons (Top-Right of Scene Window) ----------
    {
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 contentMax = ImGui::GetWindowContentRegionMax(); // relative to window
        ImVec2 contentMin = ImGui::GetWindowContentRegionMin();

        ImVec2 btnSize(30, 30);
        float padding = 10.0f;
        float spacing = 5.0f;

        // Compute start X position: right-aligned inside window content
        float startX = contentMax.x - (btnSize.x * 3 + spacing * 2) - padding;
        float startY = contentMin.y + padding;

        ImGui::SetCursorScreenPos(ImVec2(windowPos.x + startX, windowPos.y + startY));

        // Style
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.1f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.2f));

        // Button 1: Rotate
        if (ImGui::ImageButton((void*)(intptr_t)RotateIcon, btnSize))
            engineAPI->SendEvent(Events::Editor::Gizmo::ROTATE);

        ImGui::SameLine(0, spacing);

        // Button 2: Translate
        if (ImGui::ImageButton((void*)(intptr_t)TranslateIcon, btnSize))
            engineAPI->SendEvent(Events::Editor::Gizmo::TRANSLATE);

        ImGui::SameLine(0, spacing);

        // Button 3: Scale
        if (ImGui::ImageButton((void*)(intptr_t)ScaleIcon, btnSize))
            engineAPI->SendEvent(Events::Editor::Gizmo::SCALE);

        ImGui::PopStyleColor(3);
    }



    ImGui::End();
}

