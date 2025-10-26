#include "ItemsSelectionPanel.h"
#include "Engine/Core/Coordinator/Coordinator.h"

void ItemsSelectionPanel::Render()
{
    ImGui::Begin("SelectionPanel");
    if (ImGui::CollapsingHeader("Objects"))
    {
        ImGui::Indent();

        ImGui::BeginChild("ObjectList", ImVec2(0, 150), true);
        for (int entity = 0; entity < engineAPI->GetLightEntitiesAmount(); entity++)
        {
            std::string objectName = "Light " + std::to_string(entity);
            ImGui::Selectable(objectName.c_str());
            if (ImGui::IsItemClicked())
            {
                engineAPI->SetSelectedEntity(entity);
            }
            if (ImGui::BeginPopupContextWindow("EntityEditPopUp"))
            {
                if (ImGui::MenuItem("DeleteEntity"))
                {
					engineAPI->ScheduleEntityDestruction(entity);
                }
                ImGui::EndPopup();
            }
        }
        for (int entity = 11; entity <= engineAPI->GetEntitiesAmount()+10; entity++)
        {
            std::string objectName = "Entity " + std::to_string(entity);
            ImGui::Selectable(objectName.c_str());
            if (ImGui::IsItemClicked())
            {
                engineAPI->SetSelectedEntity(entity);
            }
            if (ImGui::BeginPopupContextWindow("EntityEditPopUp"))
            {
                if (ImGui::MenuItem("DeleteEntity"))
                {
                    engineAPI->ScheduleEntityDestruction(entity);
                }
                ImGui::EndPopup();
            }
        }
        ImGui::EndChild();
    }

    ImGui::End();
}
