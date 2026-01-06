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
#include <Panels/MaterialGraphPanel.h>
#include <AnimGraphEditor.h>

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

void EditorLayer::OnInit()
{
    EngineApi_->AddEventListener(Events::Editor::MaterialSystem::OPEN_MATERIAL_FILE, [&](Event& event) {
        std::unique_ptr MatherialGraph = std::make_unique<MaterialGraphPanel>();
        MatherialGraph->Init(EngineApi_, EngineApp_);

		std::string path = event.GetParam<std::string>("PATH");

		MatherialGraph->LoadMaterial(path);

        uiComponents.push_back(std::move(MatherialGraph));
        });

    EngineApi_->AddEventListener(Events::Editor::StateMachineGraph::OPEN_STATEMACHINE_FILE, [&](Event& event) {
        std::unique_ptr MatherialGraph = std::make_unique<AnimGraphEditor>();
        MatherialGraph->Init(EngineApi_, EngineApp_);

        std::string path = event.GetParam<std::string>("PATH");

        MatherialGraph->LoadGraph(path);

        uiComponents.push_back(std::move(MatherialGraph));
        });
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
