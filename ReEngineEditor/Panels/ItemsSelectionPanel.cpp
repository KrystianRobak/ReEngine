#include "Engine/Systems/UI/Panels/ItemsSelectionPanel.h"
#include "Engine/Core/Coordinator/Coordinator.h"

void ItemsSelectionPanel::Render()
{
    std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();

    ImGui::Begin("SelectionPanel");
    if (ImGui::CollapsingHeader("Objects"))
    {
        ImGui::Indent();

        ImGui::BeginChild("ObjectList", ImVec2(0, 150), true);
        for (int entity = 0; entity < coordinator->GetLightEntitiesAmount(); entity++)
        {
            std::string objectName = "Light " + std::to_string(entity);
            ImGui::Selectable(objectName.c_str());
            if (ImGui::IsItemClicked())
            {
                coordinator->SetSelectedEntity(entity);
            }

        }
        for (int entity = 11; entity <= coordinator->GetEntitiesAmount()+10; entity++)
        {
            std::string objectName = "Entity " + std::to_string(entity);
            ImGui::Selectable(objectName.c_str());
            if (ImGui::IsItemClicked())
            {
                coordinator->SetSelectedEntity(entity);
            }
        }
        ImGui::EndChild();
    }

    ImGui::End();
}
