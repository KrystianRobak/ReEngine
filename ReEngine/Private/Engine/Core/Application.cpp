#include "Engine/Core/Application.h"

#include "ReTypes.h"
#include "thread"
#include "Engine/Systems/UI/UiSystem.h"
#include <iostream>
#include "Logger.h"

#include <GL/glew.h>
#include "GLFW/glfw3.h"

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

	Commander_.Init(32);
	
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
	Renderer_->InjectCommander(std::make_shared<Commander>(Commander_));
	PhysicsSystem_ = coordinator->GetSystem("Physics3D");
	//PhysicsSystem_->InjectCommander(Commander_);
}

void Application::Update()
{
	int i = 0;
	while (true)
	{
		RenderUpdateThreadSemaphore.acquire();
		StartClock();

		auto systems = Reflection::Registry::Instance().GetAllSystems();
		for (auto system : systems)
		{
			if (std::strcmp(system->fullName,"RenderOpenGL"))
			{
				auto RenderSystem = coordinator->GetSystem(system->fullName);
				for (Entity entity : RenderSystem->GetEntities())
				{
					Commander_.IssueCommand(RenderCommand((uint32_t)i, { entity, *(Transform*)coordinator->GetComponent(entity, "Transform"), 3, 5 }));
				}
			}
		}
		GameUpdateThreadSemaphore.release();
	}

}


void Application::Render()
{

	window.Init(1280, 720, "Okno zycia", GetCoordinatorEditor());

	Renderer_->InitApi(GetCoordinatorEditor(), glfwGetCurrentContext());

	CreateUiPanels();

	window.InitUiComponets();

	while (true)
	{
		GameUpdateThreadSemaphore.acquire();

		/*glfwMakeContextCurrent(Renderer_->GetRenderContext());*/

		window.PreRender();
		
		Renderer_->Update(dt);
		std::this_thread::sleep_for(std::chrono::milliseconds(120));

		window.Render();


		window.PostRender();

		/*glfwMakeContextCurrent(nullptr);*/

		RenderUpdateThreadSemaphore.release();
		MeasureTime();
	}
		
}

void Application::PhysicsTick()
{
	using clock = std::chrono::steady_clock;
	const std::chrono::milliseconds fixedDelta(16); // ~60Hz

	while (true)
	{
		auto startTime = clock::now();

		PhysicsSystem_->Update(0.016f);
		coordinator->SwapComponentBuffers("Transform");

		auto endTime = clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

		if (elapsed < fixedDelta)
		{
			std::this_thread::sleep_for(fixedDelta - elapsed);
		}
	}
}


void Application::SetPostUpdateUI(FunctionDelegate fun)
{
	OnPostUpdateUI = std::move(fun);
}

void Application::SetUpdateUI(FunctionDelegate fun)
{
	OnUpdateUI = std::move(fun);
}

void Application::SetPreUpdateUI(FunctionDelegate fun)
{
	OnPreUpdateUI = std::move(fun);
}

void Application::SetCreateUiPanels(FunctionDelegate fun)
{
	CreateUiPanels = std::move(fun);
}

void Application::RenderEntitiesUI()
{

}

void Application::CreateCoordinator()
{
	coordinator = Coordinator::GetCoordinator();

	coordinator->Init();
}
