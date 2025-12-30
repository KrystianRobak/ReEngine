#include "PropertyPanel.h"
#include "Engine/Components/Renderable.h"
#include "Engine/Components/Gravity.h"
#include "Engine/Components/RigidBody.h"
//#include "Engine/Components/Collision.h"
#include "Engine/Components/Player.h"
#include "Engine/Components/Animated.h"
#include "Engine/Components/LightSource.h"
#include <Engine/Components/BehaviourScript.h>


inline std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;

}

void RenderComponentsMenu(std::int32_t& entity, std::bitset<32>& signature) 
{

}

void PropertyPanel::ForEachComponent(const char* header, std::vector<const Reflection::ClassInfo*> Components, std::function<void(Entity, const char*)> function)
{
    if (ImGui::CollapsingHeader(header))
    {
        for (auto component : Components)
        {
            Entity entity = engineAPI->GetSelectedEntity();

            if (ImGui::Button(component->name))
            {
                function(entity, component->name);
            }
            
        }
    }
}


void PropertyPanel::Render()
{
	ImGui::Begin("Properties");

    auto Components = Reflection::Registry::Instance().GetAllComponents();

    ForEachComponent("Add component", Components,
        [this](Entity entity, const char* name) {
            Signature signature = engineAPI->GetEntitySignature(entity);
            if (!signature.test(engineAPI->GetComponentType(name)))
            {
                engineAPI->AddComponent(entity, name);
            }
        });

    ForEachComponent("Remove component", Components,
        [this](Entity entity, const char* name) {
            Signature signature = engineAPI->GetEntitySignature(entity);
            if (signature.test(engineAPI->GetComponentType(name)))
            {
                engineAPI->RemoveComponent(entity, name);
            }
        });

    Entity entity = engineAPI->GetSelectedEntity();
    Signature signature = engineAPI->GetEntitySignature(entity);

    for (auto componentInfo : Components) {
        // Check if the current entity has this component.
        if (signature.test(engineAPI->GetComponentType(componentInfo->name))) {
            // If it does, render a collapsible header for it.
            if (ImGui::CollapsingHeader(componentInfo->name)) {
                // Get the actual component data pointer.
                // 1. Get BOTH pointers
                void* readData = engineAPI->GetComponent(entity, componentInfo->name);
                void* writeData = engineAPI->GetComponentForWrite(entity, componentInfo->name);

                // Check if we actually have two different buffers (Double Buffering Active)
                bool isDoubleBuffered = (readData != writeData && writeData != nullptr);

                for (auto& variable : componentInfo->variables) {
                    char* readVarPtr = (char*)readData + variable.offset;
                    char* writeVarPtr = (isDoubleBuffered) ? (char*)writeData + variable.offset : nullptr;

                    const char* typeName = variable.type->name;
                    const char* varName = variable.name;

                    bool valueChanged = false;

                    // --- Render UI based on READ pointer ---
                    // We use the Read pointer for the UI so the user sees the current frame's state
                    if (strcmp(typeName, "float") == 0) {
                        if (ImGui::DragFloat(varName, (float*)readVarPtr, 0.1f)) valueChanged = true;
                    }
                    else if (strcmp(typeName, "int") == 0) {
                        if (ImGui::InputInt(varName, (int*)readVarPtr)) valueChanged = true;
                    }
                    else if (strcmp(typeName, "bool") == 0) {
                        if (ImGui::Checkbox(varName, (bool*)readVarPtr)) valueChanged = true;
                    }
                    else if (strcmp(typeName, "glm::vec<3, float>") == 0 || strcmp(typeName, "glm::vec3") == 0) {
                        if (ImGui::DragFloat3(varName, (float*)readVarPtr, 0.1f)) valueChanged = true;
                    }
                    else if (strcmp(typeName, "glm::vec<4, float>") == 0 || strcmp(typeName, "glm::vec4") == 0) {
                        if (ImGui::DragFloat4(varName, (float*)readVarPtr, 0.1f)) valueChanged = true;
                    }
                    else if (strcmp(typeName, "glm::qua<float>") == 0) {
                        if (ImGui::DragFloat4(varName, (float*)readVarPtr, 0.1f)) valueChanged = true;
                    }

                    // --- FORCE SYNC: Apply change to WRITE buffer ---
                    if (valueChanged && isDoubleBuffered) {
                        // Copy the modified value from Read buffer to Write buffer
                        // We copy only the specific variable size
                        size_t varSize = variable.type->size; // Ensure ClassInfo::Variable has size! 
                        // If you don't have size in Variable struct, you need to deduce it from typeName

                        memcpy(writeVarPtr, readVarPtr, varSize);
                    }
                    
                }

            }
        }
    }

	ImGui::End();
}