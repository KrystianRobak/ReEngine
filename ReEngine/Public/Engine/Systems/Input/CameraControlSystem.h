#pragma once

#include "System/System.h"
#include "Engine/Components/Camera.h"

class Event;


class CameraControlSystem : public System
{
public:
	void Init();

	void Update(float dt);

	void BindCamera(Camera* NewCamera)
	{	
		ControlledCamera = NewCamera;
	}

private:
	std::bitset<256> mButtons;

	void InputListener(Event& event);
	Camera* ControlledCamera;
};
