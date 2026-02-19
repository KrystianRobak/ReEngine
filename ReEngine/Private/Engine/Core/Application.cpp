// Application.cpp
#include "Engine/Core/Application.h"

#include "ReTypes.h"
#include "thread"
#include "Engine/Systems/UI/UiSystem.h"
#include "GameLayer.h"
#include <iostream>
#include "Logger.h"
#include "StaticMesh.h"

#include <GL/glew.h>
#include "GLFW/glfw3.h"
#include <SkeletalMeshComponent.h>

void Application::StartClock()
{
	frameStartTime = std::chrono::steady_clock::now();
}

void Application::MeasureTime()
{
	auto frameEndTime = std::chrono::steady_clock::now();
	dt = std::chrono::duration<float>(frameEndTime - frameStartTime).count();

	float sleepDuration = targetFrameDuration - dt;
	if (sleepDuration > 0.0f)
	{
		std::this_thread::sleep_for(std::chrono::duration<float>(sleepDuration));
	}

	auto finalFrameEndTime = std::chrono::steady_clock::now();
	dt = std::chrono::duration<float>(finalFrameEndTime - frameStartTime).count();

	frameStartTime = finalFrameEndTime;

	frameTimeAccumulator += dt;
	frameCount++;
	if (frameTimeAccumulator >= 1.0f)
	{
		float fps = static_cast<float>(frameCount) / frameTimeAccumulator;
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

	systemGraph = std::make_unique<SystemGraph>(&threadPool);

	m_PendingState = ApplicationState::Editor;
	m_AppState = ApplicationState::Editor;

	{
		std::lock_guard<std::mutex> lock(initMutex);
		renderInitialized = false;
	}
}

void Application::StartGameThreads()
{
	running.store(true);
	coordinationThreadRunning.store(true);

	const int participantCount = 2;
	mSyncBarrier = std::make_unique<std::barrier<BarrierCompletion>>(
		participantCount,
		BarrierCompletion{ this }
	);

	GameThread = std::make_unique<std::thread>(std::thread(&Application::Update, this));
	RenderThread = std::make_unique<std::thread>(std::thread(&Application::Render, this));

	LOGF_INFO("Game Loop Started.");
}

void Application::StartEditorThreads()
{
	StartGameThreads();
}

void Application::InitSystems()
{
	Renderer_ = reinterpret_cast<RenderSystem*>(coordinator->GetSystem("RenderOpenGL"));
	PhysicsSystem_ = reinterpret_cast<System*>(coordinator->GetSystem("Physics3D"));

	auto systems = Reflection::Registry::Instance().GetAllSystems();

	for (auto systemInfo : systems) {
		if (std::strcmp(systemInfo->fullName, "RenderOpenGL") == 0)
			continue;

		System* runtimeSys = coordinator->GetSystem(systemInfo->fullName);
		if (runtimeSys) {
			systemGraph->AddSystem(runtimeSys);
		}
	}

	systemGraph->Build();
	LOGF_INFO("ECS Dependency Graph Built.");
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

	while (running.load())
	{
		auto start = clock::now();

		if (inputManager) inputManager->Update(dt);

		auto assetManager = coordinator->GetAssetManager();
		auto systems = Reflection::Registry::Instance().GetAllSystems();
		for (auto system : systems)
		{
			if (std::strcmp(system->fullName, "RenderOpenGL") == 0)
			{
				auto RenderSystem = coordinator->GetSystem(system->fullName);
				for (Entity entity : RenderSystem->GetEntities())
				{
					if (coordinator->GetEntitySignature(entity).test(coordinator->GetComponentType("StaticMesh")))
					{
						auto t = static_cast<Transform*>(coordinator->GetComponent(entity, "Transform"));
						auto sm = static_cast<StaticMesh*>(coordinator->GetComponent(entity, "StaticMesh"));
						if (!t || !sm) continue;

						if (!sm->MeshResource && !sm->AssetPath.empty()) {
							sm->MeshResource = assetManager->GetMesh(sm->AssetPath);
						}

						RenderPrimitive p;
						p.ModelMatrix = ReCamera::GetModelMatrix(*t);
						p.Entity = entity;
						p.MaterialId = (sm->MaterialId == -1) ? 1200 : sm->MaterialId;
						p.Mesh = sm->MeshResource;

						window->viewports["SceneViewport"]->GetCommander()->IssueCommand({ 1200, p });
					}
					if (coordinator->GetEntitySignature(entity).test(coordinator->GetComponentType("SkeletalMeshComponent")))
					{
						auto t = static_cast<Transform*>(coordinator->GetComponent(entity, "Transform"));
						auto smc = static_cast<SkeletalMeshComponent*>(coordinator->GetComponent(entity, "SkeletalMeshComponent"));
						if (!t || !smc) continue;

						if (!smc->MeshResource && !smc->AssetPath.empty()) {
							smc->MeshResource = assetManager->GetSkeletalMesh(smc->AssetPath);
						}

						RenderPrimitive p;
						p.ModelMatrix = ReCamera::GetModelMatrix(*t);
						p.Entity = entity;
						p.MaterialId = (smc->MaterialId == -1) ? 1200 : smc->MaterialId;
						p.Mesh = smc->MeshResource;
						p.FinalBoneMatrices = smc->FinalBoneMatrices;

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
				if (m_AppState == ApplicationState::Play)
				{
					systemGraph->Execute(dt);
				}
			}
		}

		coordinator->GetEpochManager()->IncrementGameEpoch();
		coordinator->ProcessPendingEntityDeletions();

		auto end = clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

		mSyncBarrier->arrive_and_wait();
	}
	mSyncBarrier->arrive_and_drop();
	LOGF_INFO("[Update Thread] Exited main loop.");
}

void Application::Render()
{
	using clock = std::chrono::steady_clock;

	if (!window)
	{
		window = Renderer_->GetWindow();
		window->Init(1920, 1080, "Okno zycia", GetCoordinatorEditor(), this);

		Renderer_->InitApi(GetCoordinatorEditor(), this, coordinator->GetAssetManager());
		Renderer_->InitRenderContext(window);

		coordinator->SendEvent(Events::Engine::LayerManager::INITIALIZED);

		{
			std::lock_guard<std::mutex> lock(initMutex);
			renderInitialized = true;
		}
		initCondition.notify_all();
	}

	const std::chrono::milliseconds fixedDelta(200);
	std::this_thread::sleep_for(fixedDelta);

	while (running.load())
	{
		mSyncBarrier->arrive_and_wait();
		auto start = clock::now();

		coordinator->GetAssetManager()->DispatchUploads();

		window->PreRender();

		for (auto& [name, viewport] : window->viewports)
		{
			viewport->Render(Renderer_);
		}

		window->Render();
		window->PostRender();

		auto end = clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

		coordinator->GetEpochManager()->IncrementRenderEpoch();
	}
	mSyncBarrier->arrive_and_drop();
	LOGF_INFO("[Render Thread] Exited main loop.");
}

void Application::PhysicsTick()
{
	using clock = std::chrono::steady_clock;

	while (running.load())
	{
		if (m_AppState == ApplicationState::Play && PhysicsSystem_)
		{
			PhysicsSystem_->Update(0.016f);
		}

		coordinator->GetEpochManager()->IncrementPhysicsEpoch();
		mSyncBarrier->arrive_and_wait();
	}

	LOGF_INFO("[Physics Thread] Exited main loop.");
}

void Application::CoordinationLoop()
{
	LOGF_INFO("[CoordinationThread] Started monitoring for recompile requests.");

	while (coordinationThreadRunning.load())
	{
		if (recompileRequested.load())
		{
			LOGF_INFO("[CoordinationThread] Recompile request detected!");
			ExecuteRecompile();
			recompileRequested.store(false);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	LOGF_INFO("[CoordinationThread] Exited.");
}

void Application::RequestRecompile()
{
	if (isRecompiling.load())
	{
		LOGF_INFO("[Recompile] Already recompiling, ignoring request.");
		return;
	}
	LOGF_INFO("[Recompile] Recompile requested from thread!");
	recompileRequested.store(true);
}

void Application::ExecuteRecompile()
{
	LOGF_INFO("[ExecuteRecompile] Starting recompile sequence...");

	isRecompiling.store(true);

	if (m_AppState.load() == ApplicationState::Play)
	{
		SetState(ApplicationState::Editor);
	}

	LOGF_INFO("[ExecuteRecompile] Waiting for threads to pause...");
	{
		std::unique_lock<std::mutex> lock(reloadMutex);
		safeToReloadCV.wait(lock, [&] { return readyForReload; });
	}

	LOGF_INFO("[ExecuteRecompile] Threads paused. Cleaning up logic systems...");

	CleanupSystems();

	LOGF_INFO("[ExecuteRecompile] Ready for DLL reload.");
	coordinator->SendEvent(Events::Application::RECOMPILE_READY);

	RestartAfterRecompile();

	LOGF_INFO("[ExecuteRecompile] Resuming threads...");
	isRecompiling.store(false);

	{
		std::lock_guard<std::mutex> lock(reloadMutex);
		readyForReload = false;
	}
	reloadCV.notify_all();

	LOGF_INFO("[ExecuteRecompile] Hot-reload complete!");
}

void Application::StopAllThreads()
{
	LOGF_INFO("[StopAllThreads] Setting running to false...");
	running.store(false);

	if (GameThread && GameThread->joinable()) {
		GameThread->join();
		GameThread.reset();
	}
	if (RenderThread && RenderThread->joinable()) {
		RenderThread->join();
		RenderThread.reset();
	}
	mSyncBarrier.reset();
}

void Application::CleanupSystems()
{
	LOGF_INFO("[CleanupSystems] Clearing system graph...");
	if (systemGraph)
	{
		systemGraph.reset();
		systemGraph = std::make_unique<SystemGraph>(&threadPool);
	}

	LOGF_INFO("[CleanupSystems] Cleanup complete (Window preserved).");
}

void Application::RestartAfterRecompile()
{
	LOGF_INFO("[RestartAfterRecompile] Re-initializing systems...");

	InitSystems();

	if (m_StateBeforeRecompile == ApplicationState::Play)
	{
		LOGF_INFO("[RestartAfterRecompile] Restoring Play mode...");
		SetState(ApplicationState::Play);
	}
}

void Application::SetPostUpdateUI(FunctionDelegate fun) { OnPostUpdateUI = std::move(fun); }
void Application::SetUpdateUI(FunctionDelegate fun) { OnUpdateUI = std::move(fun); }
void Application::SetPreUpdateUI(FunctionDelegate fun) { OnPreUpdateUI = std::move(fun); }
void Application::SetCreateUiPanels(FunctionDelegate fun) { CreateUiPanels = std::move(fun); }
void Application::RenderEntitiesUI() {}

void Application::CreateCoordinator()
{
	coordinator = Coordinator::GetCoordinator();
	coordinator->Init(&threadPool);
}

void Application::SwapAllBuffersAndNotify() noexcept
{
	auto frameEndTime = std::chrono::steady_clock::now();
	float actualFrameDuration = std::chrono::duration<float>(frameEndTime - frameStartTime).count();
	float sleepDuration = targetFrameDuration - actualFrameDuration;
	if (sleepDuration > 0.0f) std::this_thread::sleep_for(std::chrono::duration<float>(sleepDuration));
	auto finalFrameEndTime = std::chrono::steady_clock::now();
	dt = std::chrono::duration<float>(finalFrameEndTime - frameStartTime).count();
	if (dt > 0.1f) dt = 0.1f;
	frameStartTime = finalFrameEndTime;
	frameTimeAccumulator += dt;
	frameCount++;
	if (frameTimeAccumulator >= 1.0f) {
		float fps = static_cast<float>(frameCount) / frameTimeAccumulator;
		LOGF_INFO("FPS: %.2f", fps);
		frameTimeAccumulator = 0.0f;
		frameCount = 0;
	}

	if (isRecompiling.load())
	{

		{
			std::unique_lock<std::mutex> lock(reloadMutex);
			readyForReload = true;
		}
		safeToReloadCV.notify_one();

		{
			std::unique_lock<std::mutex> lock(reloadMutex);
			reloadCV.wait(lock, [&] { return !isRecompiling.load(); });
		}
	}

	ApplicationState pending = m_PendingState.load();
	ApplicationState current = m_AppState.load();

	if (pending != current)
	{
		if (current == ApplicationState::Editor && pending == ApplicationState::Play)
		{
			LOGF_INFO("Sync: Switching to PLAY mode");
			coordinator->EnterPlayMode();
			Camera* gameCamera = nullptr;
			for (Entity i = 0; i < MAX_ENTITIES; ++i) {
				if (!coordinator->IsEntityAlive(i)) continue;
				if (coordinator->HasComponent(i, "Camera")) {
					gameCamera = (Camera*)coordinator->GetComponent(i, "Camera");
					break;
				}
			}
			if (gameCamera && coordinator->GetCurrentScene()) coordinator->GetCurrentScene()->SetOverrideCamera(gameCamera);

			coordinator->SendEvent(Events::Application::CAMERA_CHANGED);
			m_AppState.store(ApplicationState::Play);

			coordinator->OnBeginSimulation();


			GetLayerManager()->AddLayerThreadSafe<GameUILayer>();
		}
		else if (current == ApplicationState::Play && pending == ApplicationState::Editor)
		{
			LOGF_INFO("Sync: Switching to EDITOR mode");
			if (coordinator->GetCurrentScene()) coordinator->GetCurrentScene()->SetOverrideCamera(nullptr);
			coordinator->SendEvent(Events::Application::CAMERA_CHANGED);
			coordinator->ExitPlayMode();
			m_AppState.store(ApplicationState::Editor);
			GetLayerManager()->RemoveLayer<GameUILayer>();
			coordinator->OnEndSimulation();
		}
		else
		{
			m_AppState.store(pending);
		}
	}

	coordinator->ProcessPendingEntityDeletions();
	coordinator->SwapComponentBuffers("Transform");
	coordinator->GetEpochManager()->IncrementGameEpoch();
	coordinator->GetEpochManager()->IncrementRenderEpoch();
}