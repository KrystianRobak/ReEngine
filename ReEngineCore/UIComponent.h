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

inline ImTextureID LoadTexture(const char* path)
{
    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 4);
    if (data)
    {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);

        return (ImTextureID)(intptr_t)tex;
    }
}

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
    ImTextureID GetTexture(const std::string& path)
    {
        if (!engineAPI) return (ImTextureID)0;

        auto assetManager = engineAPI->GetAssetManager();
        if (!assetManager) return (ImTextureID)0;

        // 1. Check if the GPU already has this texture ready
        // We cast to TextureResource because AssetManagerApi might return a base type, 
        // but based on your code, GetTextureResource returns the resource struct.
        auto* resource = assetManager->GetTextureResource(path);

        if (resource && resource->uploaded)
        {
            // Success: Return the OpenGL ID cast to ImTextureID
            return (ImTextureID)(intptr_t)resource->id;
        }

        // 2. If not found, request it to be loaded async
        // The AssetManager::loadTexture implementation checks its own cache,
        // so calling this repeatedly is safer than calling stbi_load repeatedly.
        assetManager->loadTexture(path);

        // 3. Return 0 (or a default "loading" icon ID) while waiting for async load
        return (ImTextureID)0;
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