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

        

        // [FIX 1] Iterate through all POTENTIAL IDs, not just the count.
        // If MAX_ENTITIES is huge, you should implement an iterator in Coordinator.
        // For now, assuming standard ECS size (e.g. 5000), checking IsAlive is safer.
        for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
        {
            // [FIX 2] Skip dead entities. 
            // This prevents clicking on "Ghost" entities that don't exist but fall within the count range.
            if (!engineAPI->IsEntityAlive(entity))
            {
                continue;
            }

            // [FIX 3] PushID ensures ImGui knows this Selectable is unique (crucial for events)
            ImGui::PushID(entity);

            std::string objectName = "Entity " + std::to_string(entity);

            // Check if this specific entity is the selected one for highlighting
            bool isSelected = (engineAPI->GetSelectedEntity() == entity);

            if (ImGui::Selectable(objectName.c_str(), isSelected))
            {
                engineAPI->SetSelectedEntity(entity);
            }

            // [FIX 4] Use ContextItem, not ContextWindow.
            // This attaches the popup ONLY to the "Selectable" item above.
            if (ImGui::BeginPopupContextItem("EntityCtxMenu"))
            {
                if (ImGui::MenuItem("Delete Entity"))
                {
                    engineAPI->ScheduleEntityDestruction(entity);

                    // Deselect if we just deleted the selected object
                    if (engineAPI->GetSelectedEntity() == entity)
                    {
                        engineAPI->SetSelectedEntity(MAX_ENTITIES + 1); // Set to invalid
                    }
                }
                if (ImGui::MenuItem("Save as Prefab"))
                {
                    // For MVP, save to Assets folder with a generic name.
                    // Ideally, use a proper FileDialog here.
                    std::string filename = "Assets/Entity_" + std::to_string(entity) + ".prefab";

                    if (engineAPI->SaveEntityAsPrefab(entity, filename)) {
                        std::cout << "[Editor] Saved Prefab to: " << filename << std::endl;
                        // Force FileBrowser to refresh if it's looking at "Assets"
                        // You might need an event for this: engineAPI->SendEvent(Events::Editor::REFRESH_BROWSER);
                    }
                }
                ImGui::EndPopup();
            }

            

            ImGui::PopID(); // Restore ID stack
        }
        ImGui::EndChild();
    }
    ImGui::End();
}
