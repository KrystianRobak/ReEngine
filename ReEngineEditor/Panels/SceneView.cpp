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
#include "PhysicsWorld.h"

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

    // ---------------------------------------------------------
    // [Added] RAYCAST SELECTION LOGIC
    // ---------------------------------------------------------
    // Check if mouse is hovering THIS window and Left Click occurred
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        // 1. Get Mouse Coordinates relative to the Viewport Image
        ImVec2 mousePos = ImGui::GetMousePos();
        float localX = mousePos.x - ImGui::GetWindowPos().x;
        float localY = mousePos.y - ImGui::GetWindowPos().y;

        // 2. Convert to Normalized Device Coordinates (NDC)
        // ImGui (0,0) is Top-Left. OpenGL (0,0) is Center. Y grows Up in GL.
        float ndcX = (2.0f * localX) / size.x - 1.0f;
        float ndcY = 1.0f - (2.0f * localY) / size.y;

        // 3. Get Camera Matrices (Construct manually to ensure sync with render)
        auto m_activeScene = engineAPI->GetCurrentScene();
        Camera* camera = m_activeScene->GetDefaultCamera();

        if (camera)
        {
            glm::mat4 projection = glm::perspective(glm::radians(camera->fov), size.x / size.y, 0.1f, 1000.0f);
            glm::mat4 view = glm::lookAt(camera->CameraTransform.position, camera->CameraTransform.position + camera->cameraFront, camera->cameraUp);

            // 4. Calculate Ray in World Space
            // Invert Projection to go from Clip Space -> Eye Space
            glm::vec4 rayClip = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
            glm::vec4 rayEye = glm::inverse(projection) * rayClip;
            rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0); // We want a direction, so W=0, Z points forward

            // Invert View to go from Eye Space -> World Space
            glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(view) * rayEye));

            // 5. Perform Raycast
            // We cast Application* because IApplicationApi might not strictly expose GetPhysicsWorld yet
            PhysicsWorld* physWorld = engineApp->GetPhysicsWorld();

            if (physWorld)
            {
                RaycastHit hit;
                // Cast from camera position, along calculated direction
                if (physWorld->RaycastSingle(camera->CameraTransform.position, rayWorld, 1000.0f, hit))
                {
                    // Hit found! Select the entity.
                    std::cout << "[Editor] Selected Entity ID: " << hit.entity << std::endl;
                    engineAPI->SetSelectedEntity(hit.entity);
                }
                else
                {
                    // Clicked empty space? Deselect.
                    engineAPI->SetSelectedEntity(MAX_ENTITIES + 1); // Or a specific deselect constant
                }
            }
        }
    }

    if (ImGui::BeginDragDropTarget()) {

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

        const std::string prefabPayloadID = GetDragPayloadType(FileType::Prefab);

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(prefabPayloadID.c_str())) {
            if (payload->DataSize > 0) {
                std::string nameWithPath(static_cast<const char*>(payload->Data));
                std::vector<std::string> parts = splitString(nameWithPath, '|');

                if (parts.size() == 2) {
                    std::string path = parts[1];

                    // Instantiate!
                    Entity newEntity = engineAPI->InstantiatePrefab(path);

                    // Optional: Move to mouse position if you implement Raycasting (see context)
                    // For now, it spawns at the prefab's saved location.
                    std::cout << "[Editor] Instantiated Prefab: " << path << " as Entity " << newEntity << std::endl;

                    // Select the new entity
                    engineAPI->SetSelectedEntity(newEntity);
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

