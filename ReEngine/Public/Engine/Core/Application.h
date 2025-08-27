#pragma once

#include "Engine/Core/Coordinator/Coordinator.h"

#include <chrono>
#include <random>
#include <thread>

#include "../ReEngineExport.h"

class ENGINE_API Application
{
public:
	void StartClock();
	void MeasureTime();
	void Init();

	void Update();
	void Render();
	void PhysicsTick();

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

	std::unique_ptr<std::thread> GameThread;
	std::unique_ptr<std::thread> RenderThread;
	std::unique_ptr<std::thread> PhysicsThread;

	std::shared_ptr<Coordinator> coordinator;
	System* Renderer_;
	System* PhysicsSystem_;

	bool HasRenderUpdateThreadFinished = false;

	const float targetFrameDuration = 1.0f / 60.0f;
	float frameTimeAccumulator = 0.0f;
	int frameCount = 0;
	std::chrono::steady_clock::time_point frameStartTime;
};

