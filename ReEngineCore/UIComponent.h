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


class ILayer;

class UIComponent
{
public:
	virtual void Init(Editor::IEngineEditorApi* engineAPI, IApplicationApi* engineapp)
	{
		this->engineAPI = engineAPI;
		this->engineApp = engineapp;

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

protected:
	Editor::IEngineEditorApi* engineAPI = nullptr;
	IApplicationApi* engineApp = nullptr;

    ILayer* parentLayer = nullptr;

    bool closed = false;
    bool minimized = false;
public:
    bool pendingRemove = false;
};