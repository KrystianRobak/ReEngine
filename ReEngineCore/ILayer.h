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

	virtual void OnAttach()
	{
		for (auto& component : uiComponents)
		{
			component->Init(EngineApi_);
		}
	}

	virtual void OnDetach()
	{

	}

	virtual void OnUpdate(float deltaTime)
	{
		for (auto& component : uiComponents)
		{
			component->Render();
		}
	}

	virtual void OnEvent(class Event& event) = 0;

	virtual const char* GetName() const = 0;
	virtual ~ILayer() = default;

public:
	const char* name = "Layer";

	std::vector<std::unique_ptr<UIComponent>> uiComponents;
	Editor::IEngineEditorApi* EngineApi_ = nullptr;
};