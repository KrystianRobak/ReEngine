#include <Gl/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "SceneView.h"
#include "StaticMesh.h"
#include "Transform.h"
#include <SkeletalMeshComponent.h>
#include <AssetFileFormat.h>
#include "ReScene.h"
#include "ReCamera.h"

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

void SceneView::PollInput(float currentDt)
{
    if(engineApp->GetState() != ApplicationState::Editor)
		return;

    IInputManager* inputManager = engineApp->GetInputManager();
    if (!inputManager) return;

    auto m_activeScene = engineAPI->GetCurrentScene();
    Camera* camera = m_activeScene->GetDefaultCamera();
    if (!camera) return;

    // ---------------------------------------------------------
    // 1. MOVEMENT (WASD)
    // ---------------------------------------------------------
    float dt = currentDt;
    float velocity = 10.0f * dt;

    // Calculate the Right Vector (Cross Product of Front and Up)
    glm::vec3 cameraRight = glm::normalize(glm::cross(camera->cameraFront, camera->cameraUp));

    if (inputManager->IsActionActive("Move Forward"))
        camera->CameraTransform.position += camera->cameraFront * velocity;

    if (inputManager->IsActionActive("Move Backward"))
        camera->CameraTransform.position -= camera->cameraFront * velocity;

    if (inputManager->IsActionActive("Move Right"))
        camera->CameraTransform.position += cameraRight * velocity;

    if (inputManager->IsActionActive("Move Left"))
        camera->CameraTransform.position -= cameraRight * velocity;

    // ---------------------------------------------------------
    // 2. ROTATION (Mouse Look)
    // ---------------------------------------------------------

    // Use structured bindings to get x and y from the std::pair
    auto [xpos, ypos] = inputManager->GetMousePosition();

    if (inputManager->IsActionActive("Enable Look"))
    {
        // Calculate delta (offset) from the last known position
        float xoffset = xpos - camera->lastX;
        float yoffset = camera->lastY - ypos; // Reversed: y-ranges bottom to top

        // Update last known position for the next frame
        camera->lastX = xpos;
        camera->lastY = ypos;

        // Apply sensitivity
        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        // Modify Yaw and Pitch
        camera->yaw += xoffset;
        camera->pitch += yoffset;

        // Clamp Pitch (Prevent screen flipping)
        if (camera->pitch > 89.0f)  camera->pitch = 89.0f;
        if (camera->pitch < -89.0f) camera->pitch = -89.0f;

        // Recalculate Front Vector
        // 
        glm::vec3 front;
        front.x = cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
        front.y = sin(glm::radians(camera->pitch));
        front.z = sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));

        camera->cameraFront = glm::normalize(front);
    }
    else
    {
        // Keep updating lastX/lastY even when not rotating.
        // This prevents the camera from "snapping" to an old position 
        // the moment you press the "Enable Look" button.
        camera->lastX = xpos;
        camera->lastY = ypos;
    }
}

void SceneView::Render()
{
	PollInput(0.016f); // Assuming ~60 FPS for now; ideally pass actual delta time

    ImGui::Begin("Scene");

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    size = { viewportPanelSize.x, viewportPanelSize.y };

    ImGui::Image(reinterpret_cast<void*>(viewport->GetTexture()), ImVec2{ size.x, size.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

    //// --- Drag and Drop Target: Capture path and trigger pop-up ---
    //if (ImGui::BeginDragDropTarget()) {
    //    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_STATIC_MESH")) {
    //        if (payload->DataSize > 0) {
    //            std::string nameWithPath(static_cast<const char*>(payload->Data));
    //            std::vector<std::string> parts = splitString(nameWithPath, '|');

    //            if (parts.size() == 2) {
    //                // 1. Store the data temporarily
    //                pendingImportName = parts[0];
    //                pendingImportPath = parts[1];

    //                // 2. Set the flag to show the pop-up on the next frame
    //                showImportTypePopup = true;
    //            }
    //        }
    //    }
    //    ImGui::EndDragDropTarget();
    //}

    //// --- Pop-up Logic ---
    //if (showImportTypePopup) {
    //    ImGui::OpenPopup("Select Import Type");
    //    // Keep the flag true until the user makes a choice
    //}

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

