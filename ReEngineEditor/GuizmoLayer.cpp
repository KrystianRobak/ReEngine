#include "GuizmoLayer.h"

#include <GuizmoPanel.h>

GuizmoLayer::GuizmoLayer()
{
	uiComponents.push_back(std::make_unique<GuizmoPanel>());
	name = "EditorLayer";
}

void GuizmoLayer::OnAttach()
{
	ILayer::OnAttach();
}

void GuizmoLayer::OnDetach()
{
	ILayer::OnDetach();
}

void GuizmoLayer::OnUpdate(float deltaTime)
{
	ILayer::OnUpdate(deltaTime);
}

void GuizmoLayer::OnEvent(Event& event)
{
}

const char* GuizmoLayer::GetName() const
{
	return nullptr;
}
