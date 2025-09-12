#pragma once
#include <memory>
#include "IWindow.h"

#include <string>

#include "Engine/Core/Coordinator/Event.h"
#include "UIComponent.h"

#include "Context/OpenGlContext.h"
#include "Context/UIContext.h"
#include "Context/OpenGlFrameBuffer.h"




class Window : public IWindow
{
public:
    Window() : IsRunning(true), window(nullptr)
    {
        UICtx = std::make_unique<UIContext>();
        RenderCtx = std::make_unique<OpenGlContext>();
    }

    ~Window();

    void AddUIComponent(UIComponent* UiComponent)
    {
        UIComponents.push_back(UiComponent);
	}

    bool Init(int width, int height, const std::string& title, Editor::IEngineEditorApi* EngineApi);

    void PreRender();

    void Render();

    void PostRender();

    void InitUiComponets();

    void* get_native_window() { return window; };

    void set_native_window(void* window)
    {
        this->window = (GLFWwindow*)window;
    }

    void on_mode_Changed(Event& event);

    void on_resize(int width, int height);

    void on_close();

    bool is_running() { return true; }

private:

    GLFWwindow* window;

    FrameBuffer* frameBuffer;
    // Render contexts
    std::unique_ptr<UIContext> UICtx;

    std::unique_ptr<OpenGlContext> RenderCtx;

	std::vector<UIComponent*> UIComponents;

    bool IsRunning;

    MenuType CurrentMode = MenuType::BaseMenu;

	Editor::IEngineEditorApi* EngineApi_ = nullptr;
};

