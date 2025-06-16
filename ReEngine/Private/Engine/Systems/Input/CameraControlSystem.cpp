#include "Engine/Systems/Input/CameraControlSystem.h"

#include <GLFW/glfw3.h>

#include "Engine/Components/Transform.h"
#include "Engine/Core/Coordinator/Coordinator.h"
#include "Engine/Core/Types.h"


void CameraControlSystem::Init()
{
	std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
	coordinator->AddEventListener(METHOD_LISTENER_ONE_PARAM(Events::Window::INPUT, CameraControlSystem::InputListener));

	BindCamera(coordinator->GetCamera());
}

void CameraControlSystem::Update(float dt)
{
	float speed = 0.25f;

	if (mButtons[static_cast<size_t>(InputButtons::W)])
	{
		ControlledCamera->CameraTransform.position.z -= speed;
	}
	if (mButtons[static_cast<size_t>(InputButtons::S)])
	{
		ControlledCamera->CameraTransform.position.z += speed;
	}
	if (mButtons[static_cast<size_t>(InputButtons::A)])
	{
		ControlledCamera->CameraTransform.position.x -= speed;
	}
	if (mButtons[static_cast<size_t>(InputButtons::D)])
	{
		ControlledCamera->CameraTransform.position.x += speed;
	}
	if (mButtons[static_cast<size_t>(InputButtons::Q)])
	{
		ControlledCamera->CameraTransform.position.y += speed;
	}
	if (mButtons[static_cast<size_t>(InputButtons::E)])
	{
		ControlledCamera->CameraTransform.position.y -= speed;
	}
}

void CameraControlSystem::InputListener(Event& event)
{
	int key = event.GetParam<int>("InputKey");
	int action = event.GetParam<int>("InputAction");

	// Map key to Buttons enum (this mapping depends on your key-to-enum logic)
	InputButtons button = static_cast<InputButtons>(key); // Replace with your actual mapping logic

	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		mButtons.set(static_cast<size_t>(button)); // Mark the button as pressed
	}
	else if (action == GLFW_RELEASE)
	{
		mButtons.reset(static_cast<size_t>(button)); // Mark the button as released
	}
}
