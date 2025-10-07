#pragma once

#include "ILayer.h"

class GuizmoLayer : public ILayer
{
public:
	GuizmoLayer();


	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(float deltaTime) override;
	virtual void OnEvent(class Event& event) override;

	virtual const char* GetName() const override;
	virtual ~GuizmoLayer() = default;
};

