#pragma once

#include "ReTypes.h"

class UIComponent;

namespace Editor
{
    class IEngineEditorApi;
}

class IApplicationApi
{
public:

    virtual void Init() = 0;
    virtual void Update() = 0;
	virtual void Render() = 0;
    virtual bool IsRunning() = 0;
    virtual void InitSystems() = 0;
    virtual void StartThreads() = 0;
	virtual void AddUIComponent(UIComponent* UiComponent) = 0;
    virtual Editor::IEngineEditorApi* GetCoordinatorEditor() = 0;
    virtual void SetUpdateUI(FunctionDelegate function) = 0;
    virtual void SetPostUpdateUI(FunctionDelegate function) = 0;
    virtual void SetPreUpdateUI(FunctionDelegate function) = 0;
    virtual void SetCreateUiPanels(FunctionDelegate function) = 0;
};