#pragma once

#include "Engine/Core/Coordinator/Coordinator.h"

#include "System/System.h"

class UiSystem : public System
{
public:
	void OnChangeMenu(Event &event);

	void Init();

	void Update(float dt);

private:
	int menuType;
};
