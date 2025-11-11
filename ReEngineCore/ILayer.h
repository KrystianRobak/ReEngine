#pragma once

#include <memory>
#include "UIComponent.h"

class ILayer
{
public:
	void InitEngineApi(Editor::IEngineEditorApi* engineAPI, IApplicationApi* EngineApp)
	{
		this->EngineApi_ = engineAPI;
		this->EngineApp_ = EngineApp;
		OnInit();
	}

	void InitImGuiContext(ImGuiContext* imguiContext)
	{
		ImGui::SetCurrentContext(imguiContext);
	}

	virtual void OnInit() 
	{
	
	}

	virtual void OnAttach()
	{
		for (auto& component : uiComponents)
		{
			component->Init(EngineApi_, EngineApp_);
		}
	}

	virtual void OnDetach()
	{

	}

	virtual void OnUpdate(float deltaTime)
	{
		for (auto& component : uiComponents)
		{
			if (!component->IsClosed())
				component->Render();
		}

		// Remove closed components safely
		uiComponents.erase(
			std::remove_if(uiComponents.begin(), uiComponents.end(),
				[](const std::unique_ptr<UIComponent>& comp)
				{
					return comp->pendingRemove;
				}),
			uiComponents.end()
		);
	}

	void AddUIComponent(std::unique_ptr<UIComponent> cmp)
	{
		cmp->SetLayer(this);
		uiComponents.push_back(std::move(cmp));
	}

	virtual void OnEvent(class Event& event) = 0;

	virtual const char* GetName() const = 0;
	virtual ~ILayer() = default;

public:
	const char* name = "Layer";

	std::vector<std::unique_ptr<UIComponent>> uiComponents;
	Editor::IEngineEditorApi* EngineApi_ = nullptr;
	IApplicationApi* EngineApp_ = nullptr;
};