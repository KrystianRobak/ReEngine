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
	frameStartTime = std::chrono::steady_clock::now();
}

void Application::MeasureTime()
{
	auto frameEndTime = std::chrono::steady_clock::now();
	dt = std::chrono::duration<float>(frameEndTime - frameStartTime).count();

	// Sleep to maintain target frame rate (if we are faster than target)
	float sleepDuration = targetFrameDuration - dt;
	if (sleepDuration > 0.0f)
	{
		std::this_thread::sleep_for(std::chrono::duration<float>(sleepDuration));
	}

	// New end time after sleep (accurate frame time)
	auto finalFrameEndTime = std::chrono::steady_clock::now();
	dt = std::chrono::duration<float>(finalFrameEndTime - frameStartTime).count();

	// prepare for next frame
	frameStartTime = finalFrameEndTime;

	// FPS accumulator: display every 1.0 second
	frameTimeAccumulator += dt;
	frameCount++;
	if (frameTimeAccumulator >= 1.0f)
	{
		float fps = static_cast<float>(frameCount) / frameTimeAccumulator;
		// Use your logger to print FPS
		LOGF_INFO("FPS: %.2f", fps);
		frameTimeAccumulator = 0.0f;
		frameCount = 0;
	}
}

void Application::Init() 
{
	coordinator = Coordinator::GetCoordinator();

	inputManager = std::make_unique<InputManager>();
	threadPool.Init();
	coordinator->Init(&threadPool);

	{
		std::lock_guard<std::mutex> lock(initMutex);
		renderInitialized = false;
	}
}

void Application::StartGameThreads()
{
	const int participantCount = 3; // Game + Render + Physics
	mSyncBarrier = std::make_unique<std::barrier<BarrierCompletion>>(
		participantCount,
		BarrierCompletion{ this }
	);

	running.store(true);

	// We start all threads even if in Editor mode.
	// The individual thread loops will decide whether to work or idle based on m_AppState.
	GameThread = std::make_unique<std::thread>(std::thread(&Application::Update, this));
	RenderThread = std::make_unique<std::thread>(std::thread(&Application::Render, this));
	PhysicsThread = std::make_unique<std::thread>(std::thread(&Application::PhysicsTick, this));

	LOGF_INFO("Threads started (Game Mode Ready)");
}

void Application::StartEditorThreads()
{
	// Same as GameThreads, usually we want Physics thread available for "Play" testing inside editor.
	StartGameThreads();
}

void Application::InitSystems()
{
	Renderer_ = reinterpret_cast<RenderSystem*>(coordinator->GetSystem("RenderOpenGL"));
	PhysicsSystem_ = coordinator->GetSystem("Physics3D");
}

IViewport* Application::CreateNewViewport(std::string name, int width, int height)
{
	IViewport* mainViewport = Renderer_->CreateViewport(width, height);

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


		auto start = clock::now();

		if (inputManager) inputManager->Update(dt);

		auto assetManager = coordinator->GetAssetManager();
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
						auto t = static_cast<Transform*>(coordinator->GetComponent(entity, "Transform"));
						auto sm = static_cast<StaticMesh*>(coordinator->GetComponent(entity, "StaticMesh"));
						if (!t || !sm) continue;

						// 2. Asset Logic: Ensure the component has a handle to the resource
						if (!sm->MeshResource && !sm->AssetPath.empty()) {
							sm->MeshResource = assetManager->GetMesh(sm->AssetPath);
						}

						// 3. Command Packing
						RenderPrimitive p;
						p.ModelMatrix = ReCamera::GetModelMatrix(*t);
						p.Entity = entity;
						p.MaterialId = (sm->MaterialId == -1) ? 1200 : sm->MaterialId;
						p.Mesh = sm->MeshResource; // Pass the shared_ptr

						// 4. Issue Command
						window->viewports["SceneViewport"]->GetCommander()->IssueCommand({ 1200, p });
					}
				}
			}
			else if (std::strcmp(system->fullName, "Physics3D") == 0)
			{
				continue;
			}
			else
			{
				// Only update gameplay systems if we are in Play State
				if (m_AppState == ApplicationState::Play)
				{
					auto sys = coordinator->GetSystem(system->fullName);
					sys->Update(dt);
				}
			}
		}

		coordinator->GetEpochManager()->IncrementGameEpoch();

		coordinator->ProcessPendingEntityDeletions();

		auto end = clock::now(); // End timing
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

		//LOGF_INFO("Game tick took %lld milliseconds", duration);
		
		mSyncBarrier->arrive_and_wait();
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

		auto start = clock::now();

		coordinator->GetAssetManager()->DispatchUploads();

		window->PreRender();

		for(auto& [name, viewport] : window->viewports)
		{
			viewport->Render(Renderer_);
		}

		window->Render();

		window->PostRender();

		auto end = clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

		//LOGF_INFO("Render tick took %lld milliseconds", duration);
		
		coordinator->GetEpochManager()->IncrementRenderEpoch();

		mSyncBarrier->arrive_and_wait();
	}
		
}

void Application::PhysicsTick()
{
	using clock = std::chrono::steady_clock;

	while (true)
	{
		// Only step physics if playing
		if (m_AppState == ApplicationState::Play && PhysicsSystem_)
		{
			PhysicsSystem_->Update(0.016f); // Fixed Update
		}

		coordinator->GetEpochManager()->IncrementPhysicsEpoch();

		mSyncBarrier->arrive_and_wait();
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

void Application::SwapAllBuffersAndNotify() noexcept// <-- Removed noexcept here
{
	// --- Frame Timing & FPS Logic (from MeasureTime) ---
	auto frameEndTime = std::chrono::steady_clock::now();
	dt = std::chrono::duration<float>(frameEndTime - frameStartTime).count();

	// Sleep to maintain target frame rate (if we are faster than target)
	/*
	float sleepDuration = targetFrameDuration - dt;
	if (sleepDuration > 0.0f)
	{
		std::this_thread::sleep_for(std::chrono::duration<float>(sleepDuration));
	}
	*/

	// New end time after sleep (accurate frame time)
	auto finalFrameEndTime = std::chrono::steady_clock::now();
	dt = std::chrono::duration<float>(finalFrameEndTime - frameStartTime).count();

	// prepare for next frame
	frameStartTime = finalFrameEndTime;

	// FPS accumulator: display every 1.0 second
	frameTimeAccumulator += dt;
	frameCount++;
	if (frameTimeAccumulator >= 1.0f)
	{
		float fps = static_cast<float>(frameCount) / frameTimeAccumulator;
		// Use your logger to print FPS
		LOGF_INFO("FPS: %.2f", fps);
		frameTimeAccumulator = 0.0f;
		frameCount = 0;
	}
	// --- End of Frame Timing & FPS Logic ---


	// 1. Process pending deletions *before* swapping
	coordinator->ProcessPendingEntityDeletions();

	coordinator->SwapComponentBuffers("Transform");

	// 3. Increment epochs
	coordinator->GetEpochManager()->IncrementGameEpoch();
	coordinator->GetEpochManager()->IncrementRenderEpoch();
}