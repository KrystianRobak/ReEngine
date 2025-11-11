#include "Engine/Core/Application.h"


#include "ReTypes.h"
#include "thread"
#include "Engine/Systems/UI/UiSystem.h"
#include <iostream>
#include "Logger.h"
#include "StaticMesh.h"

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

	GameThread = std::make_unique<std::thread>();
	RenderThread = std::make_unique<std::thread>();
	PhysicsThread = std::make_unique<std::thread>();



	threadPool.Init();
	coordinator->Init(&threadPool);
}

void Application::StartGameThreads()
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

void Application::StartEditorThreads()
{
	GameThread = std::make_unique<std::thread>(std::thread(&Application::Update, this));
	RenderThread = std::make_unique<std::thread>(std::thread(&Application::Render, this));

	//GameThread->join();
	LOGF_INFO("Game Thread Joined");

	//RenderThread->join();
	LOGF_INFO("Render Thread Joined");
}

void Application::InitSystems()
{
	Renderer_ = reinterpret_cast<RenderSystem*>(coordinator->GetSystem("RenderOpenGL"));
	PhysicsSystem_ = coordinator->GetSystem("Physics3D");
}

IViewport* Application::CreateNewViewport(std::string name)
{
	IViewport* mainViewport = Renderer_->CreateViewport();

	window->AddViewport(name, mainViewport);

	return mainViewport;
}

void Application::Update()
{
	using clock = std::chrono::steady_clock;

	{
		std::unique_lock<std::mutex> lock(initMutex);
		initCondition.wait(lock, [&]() { return renderInitialized; });
	}

	int i = 0;
	while (true)
	{
		RenderUpdateThreadSemaphore.acquire();

		auto start = clock::now();

		auto systems = Reflection::Registry::Instance().GetAllSystems();
		for (auto system : systems)
		{
			if (std::strcmp(system->fullName,"RenderOpenGL") == 0)
			{
				auto RenderSystem = coordinator->GetSystem(system->fullName);
				for (Entity entity : RenderSystem->GetEntities())
				{
					if (coordinator->GetEntitySignature(entity).test(coordinator->GetComponentType("StaticMesh")))
					{
						window->viewports["SceneViewport"]->GetCommander()->IssueCommand(RenderCommand((uint32_t)i, { entity, *(Transform*)coordinator->GetComponent(entity, "Transform"), 1, 5 }));
					}
				}
			}
			else if (std::strcmp(system->fullName, "Physics3D") == 0)
			{
				continue;
			}
			else
			{
				auto sys = coordinator->GetSystem(system->fullName);
				sys->Update(dt);
			}
		}

		auto& pendingMeshes = coordinator->GetAssetManager()->GetPendingMeshes();

		for (auto it = pendingMeshes.begin(); it != pendingMeshes.end(); ) {
			auto& pending = *it;

			if (pending.future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {

				std::shared_ptr<StaticMeshData> staticMesh = pending.future.get();


				coordinator->AddComponent(pending.entity, "StaticMesh");
				auto staticMeshComponent = static_cast<StaticMesh*>(coordinator->GetComponent(pending.entity, "StaticMesh"));

				staticMeshComponent->AssetPath = staticMesh->path;

				staticMeshComponent->StaticMeshHandler = staticMesh;

				it = pendingMeshes.erase(it);
			}
			else {
				++it;
			}
		}

		coordinator->GetEpochManager()->IncrementGameEpoch();

		coordinator->ProcessPendingEntityDeletions();

		auto end = clock::now(); // End timing
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

		LOGF_INFO("Game tick took %lld milliseconds", duration);

		GameUpdateThreadSemaphore.release();
	}

}


void Application::Render()
{
	using clock = std::chrono::steady_clock;

	window = Renderer_->GetWindow();

	window->Init(1920, 1080, "Okno zycia", GetCoordinatorEditor(), this);
	
	Renderer_->InitApi(GetCoordinatorEditor() ,coordinator->GetAssetManager());
	Renderer_->InitRenderContext(window);

	coordinator->SendEvent(Events::Engine::LayerManager::INITIALIZED);

	{
		std::lock_guard<std::mutex> lock(initMutex);
		renderInitialized = true;
	}
	initCondition.notify_all();

	const std::chrono::milliseconds fixedDelta(200);
	std::this_thread::sleep_for(fixedDelta); // Give some time for other threads to initialize


	while (true)
	{
		GameUpdateThreadSemaphore.acquire();

		auto start = clock::now();

		window->PreRender();

		for(auto& [name, viewport] : window->viewports)
		{
			viewport->Render(Renderer_);
		}

		window->Render();

		window->PostRender();

		auto end = clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

		LOGF_INFO("Render tick took %lld milliseconds", duration);
		
		coordinator->GetEpochManager()->IncrementRenderEpoch();

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

		coordinator->GetEpochManager()->IncrementPhysicsEpoch();

		auto endTime = clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

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

	coordinator->Init(&threadPool);
}
