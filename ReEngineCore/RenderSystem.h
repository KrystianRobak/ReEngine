#pragma once

#include "System/System.h"

class Camera;
class Commander;
class IWindow;
class IViewport;

class RenderSystem : public System
{
public:
	virtual void InitRenderContext(IWindow* window) = 0;

	virtual IWindow* GetWindow() = 0;

	virtual IViewport* CreateViewport(int width, int height) = 0;

	virtual void RenderViewport(Camera* camera, Commander* commander) = 0;

};
