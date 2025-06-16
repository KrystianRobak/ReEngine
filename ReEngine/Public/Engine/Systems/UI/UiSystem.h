#pragma once

#include "Engine/Core/Coordinator/Coordinator.h"

#include "Panels/AddingPanel.h"
#include "Panels/ControlPanel.h"
#include "Panels/FileBrowser.h"
#include "Panels/PropertyPanel.h"
#include "Panels/SceneView.h"
#include "Panels/ItemsSelectionPanel.h"

#include "../System.h"

class UiSystem : public System
{
public:
	void OnChangeMenu(Event &event);

	void Init();

	void Update(float dt);

private:
	int menuType;
};
