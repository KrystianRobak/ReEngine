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
	using clock = std::chrono::steady_clock;


	int i = 0;
	while (true)
	{
		RenderUpdateThreadSemaphore.acquire();

		auto start = clock::now();

		auto systems = Reflection::Registry::Instance().GetAllSystems();
		for (auto system : systems)
		{
			if (std::strcmp(system->fullName,"RenderOpenGL"))
			{
				auto RenderSystem = coordinator->GetSystem(system->fullName);
				for (Entity entity : RenderSystem->GetEntities())
				{
					if(coordinator->GetEntitySignature(entity).test(coordinator->GetComponentType("StaticMesh")))
						Commander_.IssueCommand(RenderCommand((uint32_t)i, { entity, *(Transform*)coordinator->GetComponent(entity, "Transform"), 3, 5 }));
				}
			}
		}

		auto& pendingMeshes = AssetManager_.GetPendingMeshes();

		for (auto it = pendingMeshes.begin(); it != pendingMeshes.end(); ) {
			auto& pending = *it;

			if (pending.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
				// Move out the loaded mesh
				std::shared_ptr<StaticMeshData> staticMesh = pending.future.get();
				// Attach StaticMesh component and assign
				coordinator->AddComponent(pending.entity, "StaticMesh");
				auto staticMeshComponent = static_cast<StaticMesh*>(coordinator->GetComponent(pending.entity, "StaticMesh"));

				staticMeshComponent->StaticMeshHandler = staticMesh;

				it = pendingMeshes.erase(it);
			}
			else {
				++it;
			}
		}

		auto end = clock::now(); // End timing
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		LOGF_INFO("Game tick took %lld milliseconds", duration);

		GameUpdateThreadSemaphore.release();
	}

}


void Application::Render()
{
	using clock = std::chrono::steady_clock;
	window.Init(1280, 720, "Okno zycia", GetCoordinatorEditor());
	
	Renderer_->InitApi(GetCoordinatorEditor(), glfwGetCurrentContext(),&AssetManager_);

	CreateUiPanels();
	window.InitUiComponets(&AssetManager_);

	while (true)
	{
		GameUpdateThreadSemaphore.acquire();

		auto start = clock::now();

		window.PreRender();
	
		Renderer_->Update(dt);

		window.Render();

		window.PostRender();

		auto end = clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		LOGF_INFO("Render tick took %lld milliseconds", duration);

		RenderUpdateThreadSemaphore.release();
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

		//LOGF_INFO("Physics tick");

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
