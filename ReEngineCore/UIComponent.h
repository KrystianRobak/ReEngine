#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "stb/stb_image.h"
#include "glm/glm.hpp"
#include <GL/glew.h>
#include "GLFW/glfw3.h"

#include "ReflectionEngine.h"
#include "Api/EngineApi/CoordinatorEditorApi.h"
#include "Api/AssetManagerApi.h"

#include <vector>
#include <iostream>
#include <string>

class UIComponent
{
public:
	virtual void Init(Editor::IEngineEditorApi* engineAPI,AssetManagerApi* AssetManger ,ImGuiContext* context)
	{
		this->engineAPI = engineAPI;
		this->assetManager = std::shared_ptr<AssetManagerApi>(AssetManger);
		this->context_ = context;
	}

	virtual void Render() = 0;

	virtual void SetTextureID(uint64_t id)
	{
		this->textureID = id;
	}

protected:
	Editor::IEngineEditorApi* engineAPI = nullptr;
	std::shared_ptr<AssetManagerApi> assetManager = nullptr;
	uint64_t textureID;
	ImGuiContext* context_;
};