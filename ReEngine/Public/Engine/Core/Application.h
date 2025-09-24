#pragma once

#include "IApplicationApi.h"

#include "Engine/Core/Coordinator/Coordinator.h"
#include <Engine/Window.h>
#include "../AssetManager.h"

#include <chrono>
#include <random>
#include <thread>

#include "../Commander.h"

#include "../ReEngineExport.h"
#include <semaphore>


class ENGINE_API Application : public IApplicationApi
{
public:
	void StartClock();
	void MeasureTime();
	void Init() override;

	void StartThreads() override;

	void InitSystems() override;

	void Update() override;
	void Render() override;
	void PhysicsTick();

	void AddUIComponent(UIComponent* UiComponent) override
	{
		window.AddUIComponent(UiComponent);
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
private:
	float dt = 0.0f;
	bool running = true;

	std::unique_ptr<std::thread> GameThread;
	std::unique_ptr<std::thread> RenderThread;
	std::unique_ptr<std::thread> PhysicsThread;

	std::shared_ptr<Coordinator> coordinator;
	System* Renderer_;
	System* PhysicsSystem_;

	Window window;

	Commander Commander_;
	AssetManager AssetManager_;

	FunctionDelegate OnUpdateUI;
	FunctionDelegate OnPostUpdateUI;
	FunctionDelegate OnPreUpdateUI;

	FunctionDelegate CreateUiPanels;

	std::binary_semaphore RenderUpdateThreadSemaphore{ 1 };

	std::binary_semaphore GameUpdateThreadSemaphore{ 0 };

	bool HasRenderUpdateThreadFinished = false;

	const float targetFrameDuration = 1.0f / 60.0f;
	float frameTimeAccumulator = 0.0f;
	int frameCount = 0;
	std::chrono::steady_clock::time_point frameStartTime;
};

