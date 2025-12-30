#include "ControlPanel.h"

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

void ControlPanel::OnInit()
{
	startIcon = GetTexture("pngs/start.png");
	pauseIcon = GetTexture("pngs/stop.png");
	recompileIcon = GetTexture("pngs/recompile.png");
}

void ControlPanel::Render()
{
    ImGui::Begin("ControlPanel");
        ImGui::BeginGroup();
        const char* items[] = {
            GetMenuType(MenuType::BaseMenu),
            GetMenuType(MenuType::AnimationMenu)
        };

        /*if (ImGui::Combo("Select Menu", &type, items, 2))
        {

        }*/
        if (ImGui::ImageButton("##playButton", (isPlaying ? (void*)(intptr_t)startIcon->id : (void*)(intptr_t)pauseIcon->id), ImVec2(20, 20)))
        {
            isPlaying = !isPlaying;
            if (isPlaying)
            {
                engineApp->SetState(ApplicationState::Editor);
            }
            else
            {
                engineApp->SetState(ApplicationState::Play);
            }
        }
			
        ImGui::SameLine();

        if (ImGui::ImageButton("recompileButton",(void*)(intptr_t)recompileIcon->id, ImVec2(20, 20)))
        {
			engineAPI->SaveScene("Assets/Scenes/AutoSaveScene.json");
        }
        ImGui::EndGroup();
    ImGui::End();
}
