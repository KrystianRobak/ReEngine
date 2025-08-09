#pragma once
#include <memory>
#include "IWindow.h"

#include <string>

#include "Panels/AnimationPanel.h"
#include "Panels/KeyframeEditorPanel.h"
#include "Context/OpenGlContext.h"
#include "Context/UIContext.h"

#include "Panels/AddingPanel.h"
#include "Panels/ControlPanel.h"
#include "Panels/FileBrowser.h"
#include "Panels/ItemsSelectionPanel.h"
#include "Panels/PropertyPanel.h"
#include "Panels/SceneView.h"


class Window : public IWindow
{
public:
    Window() : IsRunning(true), window(nullptr), application(nullptr)
    {
        UICtx = std::make_unique<UIContext>();
        RenderCtx = std::make_unique<OpenGlContext>();
        application = std::make_shared<Application>();
    }

    ~Window();

    bool Init(int width, int height, const std::string& title);

    void PreRender();

    void Render();

    void PostRender();

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
    std::shared_ptr<Application> application;

    // Render contexts
    std::unique_ptr<UIContext> UICtx;

    std::unique_ptr<OpenGlContext> RenderCtx;

    std::unique_ptr<PropertyPanel> propertyPanel;

    std::unique_ptr<ItemsSelectionPanel> itemsSelectionPanel;

    std::unique_ptr<SceneView> sceneView;

    std::unique_ptr<FileBrowser> fileBrowser;

    std::unique_ptr<AddingPanel> addingPanel;

    std::unique_ptr<ControlPanel> controlPanel;

    std::unique_ptr<AnimationPanel> animationPanel;

    std::unique_ptr<KeyframeEditorPanel> keyframeEditor;

    bool IsRunning;

    MenuType CurrentMode = MenuType::BaseMenu;
};

