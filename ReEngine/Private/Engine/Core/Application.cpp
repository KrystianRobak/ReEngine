#include "Engine/Core/Application.h"

#include "ReTypes.h"
#include "thread"
#include "Engine/Systems/UI/UiSystem.h"
#include <iostream>
#include "Logger.h"

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
	
}

void Application::StartThreads()
{
	GameThread = std::make_unique<std::thread>(std::thread(&Application::Update, this));
	RenderThread = std::make_unique<std::thread>(std::thread(&Application::Render, this));
	PhysicsThread = std::make_unique<std::thread>(std::thread(&Application::PhysicsTick, this));

	//GameThread->join();
	LOGF_INFO("Game Thread Joined");

	//RenderThread->join();
	LOGF_INFO("Render Thread Joined");

	//PhysicsThread->join();
	LOGF_INFO("Physics Thread Joined");
}

void Application::InitSystems()
{
	Renderer_ = coordinator->GetSystem("RenderOpenGL");
	PhysicsSystem_ = coordinator->GetSystem("Physics2D");
}

void Application::Update()
{
	while (true)
	{
		RenderUpdateThreadSemaphore.acquire();
		StartClock();
		auto systems = Reflection::Registry::Instance().GetAllSystems();
		for (auto system : systems)
		{
			if (system->fullName == "RendererOpenGl")
			{
				
			}
		}
		
		LOGF_INFO("Game Update Thread with dt: %f", dt);
		GameUpdateThreadSemaphore.release();
	}

}


void Application::Render()
{
	while (true)
	{
		GameUpdateThreadSemaphore.acquire();
		Renderer_->Update(dt);
		std::this_thread::sleep_for(std::chrono::milliseconds(120));
		RenderUpdateThreadSemaphore.release();
		MeasureTime();
	}
		
}


void Application::PhysicsTick()
{
	while (true)
	{
		PhysicsSystem_->Update(dt);
	}
}

void Application::RenderEntitiesUI()
{

}

void Application::CreateCoordinator()
{
	coordinator = Coordinator::GetCoordinator();

	coordinator->Init();
}
