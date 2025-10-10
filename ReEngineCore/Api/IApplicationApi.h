#pragma once

#include "ReTypes.h"

class UIComponent;
class ILayerManager;

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
    virtual void StartGameThreads() = 0;
	virtual void StartEditorThreads() = 0;
	virtual ILayerManager* GetLayerManager() = 0;
    virtual Editor::IEngineEditorApi* GetCoordinatorEditor() = 0;
    virtual void SetUpdateUI(FunctionDelegate function) = 0;
    virtual void SetPostUpdateUI(FunctionDelegate function) = 0;
    virtual void SetPreUpdateUI(FunctionDelegate function) = 0;
    virtual void SetCreateUiPanels(FunctionDelegate function) = 0;
};