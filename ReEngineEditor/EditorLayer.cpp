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

EditorLayer::EditorLayer()
{
    uiComponents.push_back(std::make_unique<SceneView>());
    uiComponents.push_back(std::make_unique<PropertyPanel>());
    uiComponents.push_back(std::make_unique<ControlPanel>());
    uiComponents.push_back(std::make_unique<ItemsSelectionPanel>());
    uiComponents.push_back(std::make_unique<SystemsManagerPanel>());
    uiComponents.push_back(std::make_unique<FileBrowser>());
    uiComponents.push_back(std::make_unique<AddingPanel>());
    name = "EditorLayer";
}

void EditorLayer::OnAttach()
{
    for (auto& component : uiComponents)
    {
        component->Init(EngineApi_);
    }
}

void EditorLayer::OnDetach()
{

}

void EditorLayer::OnUpdate(float deltaTime)
{
    for(auto& component : uiComponents)
    {
        component->Render();
	}
}

void EditorLayer::OnEvent(Event& event)
{

}

const char* EditorLayer::GetName() const
{
    return nullptr;
}
