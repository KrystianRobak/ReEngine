#include "ItemsSelectionPanel.h"
#include "Engine/Core/Coordinator/Coordinator.h"

void ItemsSelectionPanel::Render()
{
    ImGui::Begin("SelectionPanel");

    if (ImGui::CollapsingHeader("Objects"))
    {
        if (ImGui::BeginPopupContextItem("EntityAddMenu"))
        {
            if (ImGui::MenuItem("Delete Entity"))
            {
                Entity entity = engineAPI->CreateEntity();

                engineAPI->AddComponent(entity, "Transform");
            }
            ImGui::EndPopup();
        }

        ImGui::Indent();
        ImGui::BeginChild("ObjectList", ImVec2(0, 150), true);

        for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
        {

            if (!engineAPI->IsEntityAlive(entity))
            {
                continue;
            }

            ImGui::PushID(entity);

            std::string objectName = "Entity " + std::to_string(entity);

            bool isSelected = (engineAPI->GetSelectedEntity() == entity);

            if (ImGui::Selectable(objectName.c_str(), isSelected))
            {
                engineAPI->SetSelectedEntity(entity);
            }

            if (ImGui::BeginPopupContextItem("EntityCtxMenu"))
            {
                if (ImGui::MenuItem("Delete Entity"))
                {
                    engineAPI->ScheduleEntityDestruction(entity);

                    if (engineAPI->GetSelectedEntity() == entity)
                    {
                        engineAPI->SetSelectedEntity(MAX_ENTITIES + 1);
                    }
                }
                if (ImGui::MenuItem("Save as Prefab"))
                {

                    std::string filename = std::to_string(entity) + ".prefab";

                    Event e(Events::Editor::FileBrowser::SAVE_PREFAB);
                    e.SetParam<std::string>("filename", filename);
                    e.SetParam<Entity>("entity", entity);
                    engineAPI->SendEvent(e);
                }
                ImGui::EndPopup();
            }

            

            ImGui::PopID(); // Restore ID stack
        }
        ImGui::EndChild();
    }
    ImGui::End();
}
