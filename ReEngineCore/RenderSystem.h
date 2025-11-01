#pragma once

#include "Window/IWindow.h"
#include "System/System.h"

class ReCamera;
class Commander;

class RenderSystem : public System
{


public:
	virtual void InitRenderContext(GLFWwindow* window) = 0;

	virtual IWindow* GetWindow() = 0;

	virtual void RenderViewport(ReCamera* camera, Commander* commander) = 0;

public:
	GLFWwindow* renderContext = nullptr;
};
