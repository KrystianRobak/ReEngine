#pragma once

#include "UIComponent.h"
#include "imgui/ImGuizmo.h"

class GuizmoPanel : public UIComponent
{
public:
	virtual void OnInit() override;
	virtual void Render() override;

private:
	ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE mode = ImGuizmo::WORLD;
};