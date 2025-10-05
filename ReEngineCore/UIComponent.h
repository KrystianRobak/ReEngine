#pragma once

#include "imgui/imgui.h"

#include "stb/stb_image.h"
#include "glm/glm.hpp"
#include <GL/glew.h>
#include "GLFW/glfw3.h"

#include "ReflectionEngine.h"
#include "Api/EngineApi/CoordinatorEditorApi.h"
#include "ReTypes.h"

#include <vector>
#include <iostream>
#include <string>

class UIComponent
{
public:
	virtual void Init(Editor::IEngineEditorApi* engineAPI)
	{
		this->engineAPI = engineAPI;

		OnInit();
	}

	virtual void OnInit() {};

	virtual void Render() = 0;

protected:
	Editor::IEngineEditorApi* engineAPI = nullptr;

};