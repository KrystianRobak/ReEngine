#pragma once

// #include "Window/IWindow.h" // <-- REMOVED to break circular dependency
#include "System/System.h"

// Forward declarations
class IWindow;
struct Camera;
class Commander;
struct GLFWwindow; // Also good practice to forward-declare GLFWwindow

class RenderSystem : public System
{
public:
    virtual void InitRenderContext(GLFWwindow* window) = 0;

    virtual IWindow* GetWindow() = 0;

    virtual IWindow* CreateWindowViewport();

    virtual void RenderViewport(Camera* camera, Commander* commander) = 0;

public:
    GLFWwindow* renderContext = nullptr;
};