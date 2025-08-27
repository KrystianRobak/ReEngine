#include "Engine/Core/Application.h"

#include "ReTypes.h"
#include "thread"
#include "Engine/Systems/UI/UiSystem.h"
#include <iostream>

void Application::StartClock() 
{
	// Measure frame start time
	std::chrono::high_resolution_clock::now();
}

void Application::MeasureTime()
{
	// Measure frame end time
	auto frameEndTime = std::chrono::high_resolution_clock::now();
	dt = std::chrono::duration<float>(frameEndTime - frameStartTime).count();

	// Sleep to maintain target frame rate
	float sleepDuration = targetFrameDuration - dt;
	if (sleepDuration > 0)
	{
		std::this_thread::sleep_for(std::chrono::duration<float>(sleepDuration));
	}

	// Recalculate frame end time to include sleep
	auto finalFrameEndTime = std::chrono::high_resolution_clock::now();
	dt = std::chrono::duration<float>(finalFrameEndTime - frameStartTime).count();

	// Accumulate frame time for FPS calculation
	frameTimeAccumulator += dt;
	frameCount++;

	// Calculate and display FPS every second
	if (frameTimeAccumulator >= 0.1f)
	{
		float fps = frameCount / frameTimeAccumulator;
		frameTimeAccumulator = 0.0f;
		frameCount = 0;
	}
}

void Application::Init()
{
	coordinator = Coordinator::GetCoordinator();

	coordinator->Init();
	GameThread = std::make_unique<std::thread>(std::thread(&Application::Update, this));
	RenderThread = std::make_unique<std::thread>(std::thread(&Application::Render, this));
	PhysicsThread = std::make_unique<std::thread>(std::thread(&Application::PhysicsTick, this));

	//GameThread->join();
	//RenderThread->join();
	PhysicsThread->join();

}

void Application::Update()
{
	std::cout << "Update" << std::endl;
}


void Application::Render()
{
	//auto renderSystem = coordinator->GetSystem<RenderSystem>();
	std::cout << "Render" << std::endl;
	//renderSystem->Update(dt);
}


void Application::PhysicsTick()
{
	std::cout << "Physics Tick" << std::endl;
}

void Application::RenderEntitiesUI()
{

}

void Application::CreateCoordinator()
{
	coordinator = Coordinator::GetCoordinator();

	coordinator->Init();
}
