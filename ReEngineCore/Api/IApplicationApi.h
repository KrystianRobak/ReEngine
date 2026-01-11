#pragma once

#include "ReTypes.h"

class IViewport;
class UIComponent;
class ILayerManager;
class IInputManager;

namespace Editor
{
    class IEngineEditorApi;
}

enum class ApplicationState
{
    Editor,
    Play,
    Pause
};

class IApplicationApi
{
public:

    virtual void Init() = 0;
    virtual void Update() = 0;
	virtual void Render() = 0;

    virtual bool IsRunning() = 0;
    virtual void InitSystems() = 0;
    virtual void StartGameThreads() = 0;
	virtual void StartEditorThreads() = 0;

    // --- State Management ---
    virtual void SetState(ApplicationState state) = 0;
    virtual ApplicationState GetState() = 0;
    // ------------------------

    virtual IViewport* CreateNewViewport(std::string name, int width, int height) = 0;

	virtual ILayerManager* GetLayerManager() = 0;
    virtual IInputManager* GetInputManager() = 0; 
    virtual Editor::IEngineEditorApi* GetCoordinatorEditor() = 0;
    virtual void SetUpdateUI(FunctionDelegate function) = 0;
    virtual void SetPostUpdateUI(FunctionDelegate function) = 0;
    virtual void SetPreUpdateUI(FunctionDelegate function) = 0;
    virtual void SetCreateUiPanels(FunctionDelegate function) = 0;
};