#pragma once

#include "Window/IWindow.h"
#include "System/System.h"

class RenderSystem : public System
{


public:
	virtual void InitRenderContext(GLFWwindow* window) = 0;

	virtual IWindow* GetWindow() = 0;

public:
	GLFWwindow* renderContext = nullptr;
};
