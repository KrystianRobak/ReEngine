#include <Gl/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "SceneView.h"
#include "StaticMesh.h"
#include "Transform.h"
#include <SkeletalMeshComponent.h>
#include <AssetFileFormat.h>

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
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_STATIC_MESH")) {
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

    if (ImGui::BeginDragDropTarget()) {

        // 1. Detect STATIC MESH Drop
        // We use the helper from AssetFileFormat to get the correct string string "ASSET_STATIC_MESH"
        const std::string staticPayloadID = GetDragPayloadType(FileType::StaticMesh);

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(staticPayloadID.c_str())) {
            if (payload->DataSize > 0) {
                std::string nameWithPath(static_cast<const char*>(payload->Data));
                std::vector<std::string> parts = splitString(nameWithPath, '|');

                if (parts.size() == 2) {
                    std::string path = parts[1];

                    // Create Entity immediately as Static Mesh
                    Entity entity = engineAPI->CreateEntity();
                    engineAPI->AddComponent(entity, "Transform");
                    auto t = (Transform*)engineAPI->GetComponentForWrite(entity, "Transform");
                    t->scale = { 1, 1, 1 };

                    engineAPI->AddComponent(entity, "StaticMesh");
                    auto sm = (StaticMesh*)engineAPI->GetComponent(entity, "StaticMesh");
                    sm->MeshResource = engineAPI->GetAssetManager()->GetMesh(path);
                    sm->AssetPath = path;

                    engineAPI->AddComponent(entity, "BoxCollider");
                    engineAPI->AddComponent(entity, "RigidBody");
                }
            }
        }

        // 2. Detect SKELETAL MESH Drop
        // We use the helper to get "ASSET_SKELETAL_MESH"
        const std::string skeletalPayloadID = GetDragPayloadType(FileType::SkeletalMesh);

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(skeletalPayloadID.c_str())) {
            if (payload->DataSize > 0) {
                std::string nameWithPath(static_cast<const char*>(payload->Data));
                std::vector<std::string> parts = splitString(nameWithPath, '|');

                if (parts.size() == 2) {
                    std::string path = parts[1];

                    // Create Entity immediately as Skeletal Mesh
                    Entity entity = engineAPI->CreateEntity();
                    engineAPI->AddComponent(entity, "Transform");
                    auto t = (Transform*)engineAPI->GetComponentForWrite(entity, "Transform");
                    t->scale = { 1, 1, 1 };

                    engineAPI->AddComponent(entity, "SkeletalMeshComponent");
                    auto smc = (SkeletalMeshComponent*)engineAPI->GetComponent(entity, "SkeletalMeshComponent");
                    smc->MeshResource = engineAPI->GetAssetManager()->GetSkeletalMesh(path);
                    smc->AssetPath = path;

                    engineAPI->AddComponent(entity, "BoxCollider");
                    engineAPI->AddComponent(entity, "RigidBody");
                }
            }
        }

        ImGui::EndDragDropTarget();
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
        if (ImGui::ImageButton((void*)(intptr_t)RotateIcon->id, btnSize))
            engineAPI->SendEvent(Events::Editor::Gizmo::ROTATE);

        ImGui::SameLine(0, spacing);

        // Button 2: Translate
        if (ImGui::ImageButton((void*)(intptr_t)TranslateIcon->id, btnSize))
            engineAPI->SendEvent(Events::Editor::Gizmo::TRANSLATE);

        ImGui::SameLine(0, spacing);

        // Button 3: Scale
        if (ImGui::ImageButton((void*)(intptr_t)ScaleIcon->id, btnSize))
            engineAPI->SendEvent(Events::Editor::Gizmo::SCALE);

        ImGui::PopStyleColor(3);
    }



    ImGui::End();
}

