#include "EditorLayer.h"

#include "Panels/AddingPanel.h"
#include "Panels/AnimationPanel.h"
#include "Panels/ControlPanel.h"
#include "Panels/FileBrowser.h"
#include "Panels/ItemsSelectionPanel.h"
#include "Panels/KeyframeEditorPanel.h"
#include "Panels/PropertyPanel.h"
#include "Panels/SceneView.h"
#include "Panels/SystemsManagerPanel.h"
#include "SceneSettings.h"

EditorLayer::EditorLayer()
{
    uiComponents.push_back(std::make_unique<SceneView>());
    uiComponents.push_back(std::make_unique<PropertyPanel>());
    uiComponents.push_back(std::make_unique<ControlPanel>());
    uiComponents.push_back(std::make_unique<ItemsSelectionPanel>());
    uiComponents.push_back(std::make_unique<SystemsManagerPanel>());
    uiComponents.push_back(std::make_unique<FileBrowser>());
    uiComponents.push_back(std::make_unique<AddingPanel>());
    uiComponents.push_back(std::make_unique<SceneSettings>());
    name = "EditorLayer";
}

void EditorLayer::OnAttach()
{
    ILayer::OnAttach();
}

void EditorLayer::OnDetach()
{
	ILayer::OnDetach();
}

void EditorLayer::OnUpdate(float deltaTime)
{
	ILayer::OnUpdate(deltaTime);
}

void EditorLayer::OnEvent(Event& event)
{

}

const char* EditorLayer::GetName() const
{
    return nullptr;
}
