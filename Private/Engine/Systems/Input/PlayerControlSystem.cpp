#include "../Public/Engine/Systems/Input/PlayerControlSystem.h"

#include "../Public/Engine/Components/Player.h"
#include "../Public/Engine/Components/Thrust.h"
#include "../Public/Engine/Components/Transform.h"
#include "../Public/Engine/Core/Coordinator/Coordinator.h"


void PlayerControlSystem::Init()
{
	std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
	coordinator->AddEventListener(METHOD_LISTENER_ONE_PARAM(Events::Window::INPUT, PlayerControlSystem::InputListener));
}

void PlayerControlSystem::Update(float dt)
{
	/*for (auto& entity : mEntities)
	{
		auto& transform = coordinator->Get<Transform>(entity);


		if (mButtons.test(static_cast<std::size_t>(InputButtons::W)))
		{
			transform.position.z += (dt * 10.0f);
		}

		else if (mButtons.test(static_cast<std::size_t>(InputButtons::S)))
		{
			transform.position.z -= (dt * 10.0f);
		}


		if (mButtons.test(static_cast<std::size_t>(InputButtons::Q)))
		{
			transform.position.y += (dt * 10.0f);
		}

		else if (mButtons.test(static_cast<std::size_t>(InputButtons::E)))
		{
			transform.position.y -= (dt * 10.0f);
		}


		if (mButtons.test(static_cast<std::size_t>(InputButtons::A)))
		{
			transform.position.x += (dt * 10.0f);
		}

		else if (mButtons.test(static_cast<std::size_t>(InputButtons::D)))
		{
			transform.position.x -= (dt * 10.0f);
		}
	}*/
}

void PlayerControlSystem::InputListener(Event& event)
{
	//mButtons = event.GetParam<std::bitset<8>>(Events::Window::Input::INPUT);
}
