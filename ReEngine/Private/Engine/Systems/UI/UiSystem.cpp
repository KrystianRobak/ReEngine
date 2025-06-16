#include "Engine/Systems/UI/UiSystem.h"

#include <iostream>
#include <ostream>

void UiSystem::OnChangeMenu(Event& event)
{
	menuType = event.GetParam<int>("MenuType");
}


void UiSystem::Init()
{
	std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
	coordinator->AddEventListener(METHOD_LISTENER_ONE_PARAM(Events::Application::MENU_CHANGED, UiSystem::OnChangeMenu));
}

void UiSystem::Update(float dt)
{
	std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();

	switch (menuType)
	{
		case MenuType::BaseMenu:
			break;
		case MenuType::AnimationMenu:
			break;
	}


	/*
	for (auto const& entity : mEntities)
	{
		coordinator->
	}*/
}