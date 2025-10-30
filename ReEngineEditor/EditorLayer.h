#pragma once
#include "ILayer.h"

class EditorLayer : public ILayer
{
public:
	EditorLayer();
	
	virtual void OnInit() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(float deltaTime) override;
	virtual void OnEvent(class Event& event) override;

	virtual const char* GetName() const override;
	virtual ~EditorLayer() = default;
};

