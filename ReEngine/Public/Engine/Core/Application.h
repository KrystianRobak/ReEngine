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
#include "../Components/Animated.h"
#include <Engine/Components/BehaviourScript.h>

#include "Engine/Core/Coordinator/Coordinator.h"

#include "Engine/Systems/Input/CameraControlSystem.h"
#include "Engine/Systems/Physics/PhysicsSystem.h"
#include "Engine/Systems/Input/PlayerControlSystem.h"
#include "Engine/Systems/Render/RenderSystem.h"
#include "Engine/Systems/Animation/AnimationSystem.h"
#include "Engine/Systems/Behaviour/BehaviourSystem.h"

#include <chrono>
#include <random>

#include "../ReEngineExport.h"

class ENGINE_API Application
{
public:
	void Init();
	void Update();
	void Render();

	void RenderEntitiesUI();

	void CreateCoordinator();

	void ToggleApplication()
	{
		//coordinator->GetSystem<AnimationSystem>()->CurrentFrame = 0;
		//coordinator->GetSystem<AnimationSystem>()->LastFrame = 0;
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

