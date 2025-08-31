#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "stb/stb_image.h"

#include <GL/glew.h>
#include "GLFW/glfw3.h"

#include "ReflectionEngine.h"
#include "EngineApi/CoordinatorEditorApi.h"

#include <vector>
#include <iostream>
#include <string>

class UIComponent
{
public:
	virtual void Init(Editor::IEngineEditorApi* engineAPI)
	{
		this->engineAPI = engineAPI;
	}

	virtual void Render() = 0;

protected:
	Editor::IEngineEditorApi* engineAPI = nullptr;
};