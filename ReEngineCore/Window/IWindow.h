#pragma once
#include <string>

#include "ILayerManager.h"
#include "UIComponent.h"
#include "IViewport.h" // <-- Now the base class
#include <memory>
#include <vector> // <-- ADDED for std::vector


class Event;

// Editor::IEngineEditorApi is not defined, forward-declaring
namespace Editor { class IEngineEditorApi; }
// MenuType is not defined, assuming enum
//enum MenuType { BaseMenu, Animation, Free };

class IWindow : protected IViewport // <-- ADDED inheritance
{
public:
    // Add protected constructor to chain to IViewport's constructor
protected:
    IWindow() : IViewport() {}

public:
    virtual void* get_native_window() = 0;
    virtual void set_native_window(void* window) = 0;
    virtual void on_resize(int width, int height) = 0;
    virtual void on_close() = 0;

    ILayerManager* GetLayerManager() { return LayerManager_.get(); }

    // Concrete Init must call IViewport::Init(width, height)
    virtual bool Init(int width, int height, const std::string& title, Editor::IEngineEditorApi* EngineApi) = 0;

    // Overriding base class methods
    virtual void PreRender() override = 0;
    virtual void Render(RenderSystem* renderer) = 0; // This is the window's main render loop
    virtual void PostRender() override = 0;

    virtual void on_mode_Changed(Event& event) = 0;

    virtual bool is_running() { return true; }

    // int width;  <-- REMOVED, inherited from IViewport
    // int height; <-- REMOVED, inherited from IViewport
    std::string title;

    // This vector now holds *sub-viewports* (e.g., editor panels)
    // The IWindow *is* the main viewport.
    std::vector<std::unique_ptr<IViewport>> viewports;

    std::unique_ptr<ILayerManager> LayerManager_;
    MenuType CurrentMode = MenuType::BaseMenu;
    Editor::IEngineEditorApi* EngineApi_ = nullptr;
};