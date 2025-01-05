#include "../Public/Engine/Core/Application.h"

#include "../Public/Engine/Core/Types.h"
#include "thread"
#include "Engine/Systems/UI/UiSystem.h"

void Application::Init()
{
	coordinator = Coordinator::GetCoordinator();

	coordinator->AddEventListener(METHOD_LISTENER_NO_PARAM(Events::Application::TOGGLE, Application::ToggleApplication));

	coordinator->RegisterComponent<Camera>();
	coordinator->RegisterComponent<Gravity>();
	coordinator->RegisterComponent<Player>();
	coordinator->RegisterComponent<Renderable>();
	coordinator->RegisterComponent<RigidBody>();
	coordinator->RegisterComponent<Thrust>();
	coordinator->RegisterComponent<Transform>();
	coordinator->RegisterComponent<Collision>();
	coordinator->RegisterComponent<StaticMesh>();
	coordinator->RegisterComponent<LightSource>();

	auto physicsSystem = coordinator->RegisterSystem<PhysicsSystem>();
	{
		Signature signature;
		signature.set(coordinator->GetComponentType<Gravity>());
		signature.set(coordinator->GetComponentType<RigidBody>());
		signature.set(coordinator->GetComponentType<Transform>());
		//signature.set(coordinator->GetComponentType<Collider>());
		coordinator->SetSystemSignature<PhysicsSystem>(signature);
	}

	physicsSystem->Init();

	auto cameraControlSystem = coordinator->RegisterSystem<CameraControlSystem>();
	{
		Signature signature;
		signature.set(coordinator->GetComponentType<Camera>());
		signature.set(coordinator->GetComponentType<Transform>());
		coordinator->SetSystemSignature<CameraControlSystem>(signature);
	}

	cameraControlSystem->Init();


	auto playerControlSystem = coordinator->RegisterSystem<PlayerControlSystem>();
	{
		Signature signature;
		signature.set(coordinator->GetComponentType<Player>());
		signature.set(coordinator->GetComponentType<Transform>());
		coordinator->SetSystemSignature<PlayerControlSystem>(signature);
	}

	playerControlSystem->Init();


	auto renderSystem = coordinator->RegisterSystem<RenderSystem>();
	{
		Signature signature;

		coordinator->SetSystemSignature<RenderSystem>(signature);
	}

	renderSystem->Init();

	auto uiSystem = coordinator->RegisterSystem<UiSystem>();
	{
		Signature signature;

		coordinator->SetSystemSignature<RenderSystem>(signature);
	}

	uiSystem->Init();

	running = false;
}

void Application::Update()
{
	auto playerControlSystem = coordinator->GetSystem<PlayerControlSystem>();
	auto cameraControlSystem = coordinator->GetSystem<CameraControlSystem>();
	auto physicsSystem = coordinator->GetSystem<PhysicsSystem>();

	const float targetFrameDuration = 1.0f / 1000.0f; // Targeting 165 FPS
	static float frameTimeAccumulator = 0.0f;
	static int frameCount = 0;

	// Measure frame start time
	auto frameStartTime = std::chrono::high_resolution_clock::now();

	// Update systems
	playerControlSystem->Update(dt);
	cameraControlSystem->Update(dt);
	physicsSystem->Update(dt);

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
	if (frameTimeAccumulator >= 0.1f) // Every 1 second
	{
		float fps = frameCount / frameTimeAccumulator;
		//std::cout << "FPS: " << fps << std::endl;
		frameTimeAccumulator = 0.0f;
		frameCount = 0;
	}
}


void Application::Render()
{
	auto renderSystem = coordinator->GetSystem<RenderSystem>();

	renderSystem->Update(dt);
}


void Application::RenderEntitiesUI()
{

}
