#include <Gl/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Window.h"
#include "SceneView.h"


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

    frameBuffer->create_buffers((int32_t)size.x, (int32_t)size.y);
}

void SceneView::Render()
{
    frameBuffer->bind();

    application->Render();

    frameBuffer->unbind();
    
    
    if(application->IsRunning())
        application->Update();


    ImGui::Begin("Scene");

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    size = { viewportPanelSize.x, viewportPanelSize.y };

    uint64_t textureID = frameBuffer->get_texture();
    ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ size.x, size.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DIRECTORY_ENTRY")) {
            if (payload->DataSize > 0) {
                std::string nameWithPath(static_cast<const char*>(payload->Data));
                std::vector<std::string> parts = splitString(nameWithPath, '|'); 
                if (parts.size() == 2) {
                    std::string name = parts[0];
                    std::string path = parts[1];
                    
                    std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();

                    Entity entity = coordinator->CreateEntity();
                }
            }
        } 
        ImGui::EndDragDropTarget();
    }

    ImGui::End();
}

