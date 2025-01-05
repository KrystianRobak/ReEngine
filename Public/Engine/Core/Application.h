#pragma once

#include "../Components/Gravity.h"
#include "../Components/RigidBody.h"
#include "../Components/Renderable.h"
#include "../Components/Thrust.h"
#include "../Components/Transform.h"
#include "../Components/Collision.h"
#include "../Components/Camera.h"
#include "../Components/Player.h"
#include "../Components/Thrust.h"
#include "../Components/LightSource.h"
#include "../Components/StaticMesh.h"

#include "../Public/Engine/Core/Coordinator/Coordinator.h"

#include "../Public/Engine/Systems/Input/CameraControlSystem.h"
#include "../Public/Engine/Systems/Physics/PhysicsSystem.h"
#include "../Public/Engine/Systems/Input/PlayerControlSystem.h"
#include "../Public/Engine/Systems/Render/RenderSystem.h"

#include "WindowManager.h"
#include <chrono>
#include <random>


class Application
{
public:
	void Init();
	void Update();
	void Render();

	void RenderEntitiesUI();

	void ToggleApplication()
	{
		this->running = !this->running;
	}

	bool IsRunning()
	{
		return running;
	}
private:
	float dt = 0.0f;
	bool running;
	std::shared_ptr<Coordinator> coordinator;
};

