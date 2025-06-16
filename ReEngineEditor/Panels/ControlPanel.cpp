#include "Engine/Systems/UI/Panels/ControlPanel.h"



inline const char* GetMenuType(const MenuType menu) {
    switch (menu)
    {
        case MenuType::BaseMenu:
            return "Base Menu";
        case MenuType::AnimationMenu:
            return "Animation Menu";
    }

    return "Null";
}

void ControlPanel::Render()
{
    ImGui::Begin("ControlPanel");
        ImGui::BeginGroup();
        const char* items[] = {
            GetMenuType(MenuType::BaseMenu),
            GetMenuType(MenuType::AnimationMenu)
        };

        if (ImGui::Combo("Select Menu", &type, items, 2))
        {
            std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();

            Event KeyEvent(Events::Application::MENU_CHANGED);
            KeyEvent.SetParam<int>("MenuType", type);
            coordinator->SendEvent(KeyEvent);
        }
        if (ImGui::Button("Start/Pause"))
        {
            std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
            coordinator->SendEvent(Events::Application::TOGGLE);
        }
        //ImGui::EndGroup();
        //ImGui::BeginGroup();
        if (ImGui::Button("Recompile shader"))
        {
            std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
            coordinator->SendEvent(Events::Application::RECOMPILE_SHADER);
        }
        ImGui::EndGroup();
    ImGui::End();
}
