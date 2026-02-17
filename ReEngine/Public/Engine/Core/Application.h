#pragma once

#include "api/IApplicationApi.h"

#include "Engine/Core/Coordinator/Coordinator.h"
#include "Window/IWindow.h"
#include "IViewport.h"

#include "AssetManager.h"

#include <chrono>
#include <random>
#include <thread>

#include "Commander.h"

#include "ReEngineExport.h"
#include <semaphore>
#include <RenderSystem.h>
#include <functional>
#include <barrier>
#include "InputManager.h"
#include <SystemGraph.h>
#include <PhysicsWorld.h>

class ENGINE_API Application : public IApplicationApi
{
public:
	Application() : window(nullptr)
	{
	}

	void StartClock();
	void MeasureTime();
	void Init() override;

	void StartGameThreads() override;
	void StartEditorThreads() override;
	void InitSystems() override;

	IViewport* CreateNewViewport(std::string name, int width, int height) override;

	void Update() override;
	void Render() override;
	void PhysicsTick();

	void SetState(ApplicationState newState) override
	{
		m_PendingState.store(newState);
	}

	ApplicationState GetState() override
	{
		return m_AppState;
	}

	ILayerManager* GetLayerManager() override
	{
		return window->GetLayerManager();
	}

	IInputManager* GetInputManager() override
	{
		return inputManager.get();
	}

	PhysicsWorld* GetPhysicsWorld()
	{
		return dynamic_cast<PhysicsWorld*>(PhysicsSystem_);
	}

	void SetPostUpdateUI(FunctionDelegate fun) override;

	void SetUpdateUI(FunctionDelegate fun) override;

	void SetPreUpdateUI(FunctionDelegate fun) override;

	void SetCreateUiPanels(FunctionDelegate fun) override;

	void RenderEntitiesUI();

	void CreateCoordinator();

	Editor::IEngineEditorApi* GetCoordinatorEditor() override
	{
		return static_cast<Editor::IEngineEditorApi*>(coordinator.get());
	}

	void ToggleApplication()
	{
		this->running = !this->running;
	}

	bool IsRunning() override
	{
		return running;
	}

	void RequestRecompile() override;
	void RestartAfterRecompile() override;
	bool IsRecompiling() const { return isRecompiling.load(); }

private:
	void StopAllThreads();
	void CleanupSystems();
	void ExecuteRecompile();
	void CoordinationLoop() override;

	void SwapAllBuffersAndNotify() noexcept;

private:
	struct BarrierCompletion
	{
		Application* self;

		void operator()() noexcept {
			self->SwapAllBuffersAndNotify();
		}
	};

	float dt = 0.0f;
	std::atomic<bool> running{ true };
	std::atomic<bool> isRecompiling{ false };
	std::atomic<bool> recompileRequested{ false };
	std::atomic<bool> coordinationThreadRunning{ true };

	std::mutex reloadMutex;
	std::condition_variable reloadCV;
	std::condition_variable safeToReloadCV;
	bool readyForReload = false;

	std::atomic<ApplicationState> m_AppState{ ApplicationState::Editor };
	std::atomic<ApplicationState> m_PendingState{ ApplicationState::Editor };
	ApplicationState m_StateBeforeRecompile{ ApplicationState::Editor };

	std::mutex initMutex;
	std::condition_variable initCondition;
	bool renderInitialized = false;

	std::unique_ptr<std::thread> GameThread;
	std::unique_ptr<std::thread> RenderThread;
	std::unique_ptr<std::thread> CoordinationThread;

	std::shared_ptr<Coordinator> coordinator;
	RenderSystem* Renderer_;
	System* PhysicsSystem_;

	std::unique_ptr<SystemGraph> systemGraph;
	std::unique_ptr<InputManager> inputManager;
	IWindow* window;
	Commander Commander_;
	ThreadPool threadPool;

	FunctionDelegate OnUpdateUI;
	FunctionDelegate OnPostUpdateUI;
	FunctionDelegate OnPreUpdateUI;
	FunctionDelegate CreateUiPanels;

	std::unique_ptr<std::barrier<BarrierCompletion>> mSyncBarrier;
	const int mSyncThreadCount = 2;
	bool HasRenderUpdateThreadFinished = false;

	const float targetFrameDuration = 1.0f / 60.0f;
	float frameTimeAccumulator = 0.0f;
	int frameCount = 0;
	std::chrono::steady_clock::time_point frameStartTime;
};