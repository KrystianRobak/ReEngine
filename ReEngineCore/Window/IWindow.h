#pragma once
#include <string>

#include "ILayerManager.h"
#include "UIComponent.h"
#include <memory>
#include <map>

class Event;
class IViewport;

class IWindow
{
public:

    void AddViewport(const std::string& name, IViewport* viewport)
    {
        viewports[name] = viewport;
	}

    virtual void* get_native_window() = 0;

    virtual void set_native_window(void* window) = 0;

    virtual void on_resize(int width, int height) = 0;

    virtual void on_close() = 0;

	ILayerManager* GetLayerManager() { return LayerManager_.get(); }

    virtual bool Init(int width, int height, const std::string& title, Editor::IEngineEditorApi* EngineApi, IApplicationApi* ApplicationApi) = 0;

    virtual void PreRender() = 0;

    virtual void Render() = 0;

    virtual void PostRender() = 0;

    virtual void on_mode_Changed(Event& event) = 0;

    virtual bool is_running() { return true; }

public:

    int width;
    int height;
    std::string title;

    std::map<std::string, IViewport*> viewports;

    std::unique_ptr<ILayerManager> LayerManager_;

    MenuType CurrentMode = MenuType::BaseMenu;

    Editor::IEngineEditorApi* EngineApi_ = nullptr;
    IApplicationApi* ApplicationApi_ = nullptr;
};