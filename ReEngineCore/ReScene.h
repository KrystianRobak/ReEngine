#pragma once

#include "ReTypes.h"

class Camera;

class ReScene
{
public:
	ReScene(const std::string& name, Camera* camera)
		: name_(name), defaultCamera(camera)
	{

	}

	Camera* GetDefaultCamera() const { return defaultCamera; }

	std::string GetName() { return name_; };

private:
	std::string name_;
	Camera* defaultCamera;
};
