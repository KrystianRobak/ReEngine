#pragma once

#include "imgui/imgui.h"
#include <imgui/imgui_internal.h>

#include "stb/stb_image.h"
#include "glm/glm.hpp"
#include <GL/glew.h>
#include "GLFW/glfw3.h"

#include "ReflectionEngine.h"
#include "Api/EngineApi/CoordinatorEditorApi.h"
#include "api/IApplicationApi.h"

#include "ReTypes.h"
#include "Event.h"

#include <memory>
#include <vector>
#include <iostream>
#include <string>
#include "TextureData.h"

#include "AssetFileFormat.h"
#include <map>
#include <filesystem>

class ILayer;

class UIComponent
{
public:
	virtual void Init(Editor::IEngineEditorApi* engineAPI, IApplicationApi* engineapp)
	{
		this->engineAPI = engineAPI;
		this->engineApp = engineapp;

        icons[FileType::Folder] = GetTexture("icons/folder.retex");
        icons[FileType::Unknown] = GetTexture("icons/file.retex");
        icons[FileType::Code] = GetTexture("icons/code.retex");
        icons[FileType::StaticMesh] = GetTexture("icons/mesh.retex");
        icons[FileType::SkeletalMesh] = GetTexture("icons/skeleton.retex");
        icons[FileType::Texture] = GetTexture("icons/texture.retex");
        icons[FileType::Material] = GetTexture("icons/material.retex");
        icons[FileType::Scene] = GetTexture("icons/scene.retex");
        icons[FileType::Animation] = GetTexture("icons/animation.retex");
        icons[FileType::Prefab] = GetTexture("icons/file.retex");

		OnInit();
	}

	virtual void OnInit() {};

	virtual void Render() = 0;

    void SetLayer(ILayer* layer) { parentLayer = layer; }

    bool IsClosed() const { return closed; }
    bool IsMinimized() const { return minimized; }

protected:
    std::shared_ptr<TextureResource> GetTexture(const std::string& path)
    {
        if (!engineAPI) return nullptr;

        auto assetManager = engineAPI->GetAssetManager();
        if (!assetManager) return nullptr;

        auto resource = assetManager->GetTexture(path);

        return resource;
    }

    bool RenderWindowTopBar()
    {
        // Check if window is docked
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window && window->DockId != 0)
        {
            // If docked → still allow minimize behavior,
            // but DO NOT show close/minimize buttons.
            if (minimized)
            {
                // Keep titlebar visible but content hidden
                return false;
            }

            return true; // Render content normally
        }

        // --- Undocked: Show Minimize & Close Buttons ---
        ImGui::SameLine(ImGui::GetWindowWidth() - 60);

        if (ImGui::Button("-"))
            minimized = !minimized;

        ImGui::SameLine();

        if (ImGui::Button("X"))
        {
            closed = true;
            pendingRemove = true;

            return false;
        }

        ImGui::Separator();

        return !minimized;
    }

    bool DrawAssetSlot(const char* label, std::string& propertyValue, const char* payloadType, FileType iconType)
    {
        bool valueChanged = false;
        ImGui::PushID(label);

        ImGui::Text(label);

        // 1. Determine Display Name
        std::string displayName = "Empty";
        if (!propertyValue.empty()) {
            displayName = std::filesystem::path(propertyValue).stem().string();
        }

        // 2. Icon Button
        ImTextureID iconID = (void*)(intptr_t)icons[iconType]->id;

        // Style the button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // Transparent background
        if (ImGui::ImageButton(iconID, ImVec2(64, 64))) {
            // Optional: Click to highlight in browser
            if (!propertyValue.empty()) {
                Event e(Events::Editor::FileBrowser::LOCATE_FILE);
                e.SetParam<std::string>("PATH", propertyValue);
                engineAPI->SendEvent(e);
            }
        }
        ImGui::PopStyleColor();

        // 3. Drag & Drop Target
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType)) {
                std::string payloadData = static_cast<const char*>(payload->Data);
                size_t splitPos = payloadData.find('|');
                if (splitPos != std::string::npos) {
                    std::string path = payloadData.substr(splitPos + 1);

                    // Only update if actually different
                    if (propertyValue != path) {
                        propertyValue = path;
                        valueChanged = true;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::SameLine();

        // 4. Info & Controls Group
        ImGui::BeginGroup();
        {
            ImGui::TextWrapped("%s", displayName.c_str());

            if (ImGui::Button("Clear", ImVec2(60, 0))) {
                propertyValue = "";
                valueChanged = true;
            }

            if (ImGui::Button("Find", ImVec2(60, 0))) {
                if (!propertyValue.empty()) {
                    Event e(Events::Editor::FileBrowser::LOCATE_FILE);
                    e.SetParam<std::string>("PATH", propertyValue);
                    engineAPI->SendEvent(e);
                }
            }
        }
        ImGui::EndGroup();

        ImGui::PopID();
        return valueChanged;
    }


protected:
	Editor::IEngineEditorApi* engineAPI = nullptr;
	IApplicationApi* engineApp = nullptr;

    ILayer* parentLayer = nullptr;

    std::map<FileType, std::shared_ptr<TextureResource>> icons;

    bool closed = false;
    bool minimized = false;
public:
    bool pendingRemove = false;
};