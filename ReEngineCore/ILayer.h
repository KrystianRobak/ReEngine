#pragma once

#include <memory>
#include "UIComponent.h"

class ILayer
{
public:
	void InitEngineApi(Editor::IEngineEditorApi* engineAPI)
	{
		this->EngineApi_ = engineAPI;
	}

	void InitImGuiContext(ImGuiContext* imguiContext)
	{
		ImGui::SetCurrentContext(imguiContext);
	}

	virtual void OnAttach() = 0;
	virtual void OnDetach() = 0;
	virtual void OnUpdate(float deltaTime) = 0;
	virtual void OnEvent(class Event& event) = 0;

	virtual const char* GetName() const = 0;
	virtual ~ILayer() = default;

public:
	const char* name = "Layer";

	std::vector<std::unique_ptr<UIComponent>> uiComponents;
	Editor::IEngineEditorApi* EngineApi_ = nullptr;
};