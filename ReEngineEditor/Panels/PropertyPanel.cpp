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

    //{
    //    if (!signature.test(coordinator->GetComponentType<Transform>()))
    //    {
    //        if (ImGui::Button("Transform"))
    //        {
    //            coordinator->AddComponent(
    //                entity,
    //                Transform{
    //                    .position = glm::vec3(1, 1, -30),
    //                    .rotation = glm::vec3(0, 0.1, 0),
    //                    .scale = glm::vec3(3, 3, 3)
    //                });
    //        }
    //    }
    //    if (!signature.test(coordinator->GetComponentType<BehaviourScript>()))
    //    {
    //        if (ImGui::Button("BehaviourScript"))
    //        {
    //            coordinator->AddComponent(
    //                entity,
    //                BehaviourScript{}
    //            );
    //        }
    //    }
    //    if (!signature.test(coordinator->GetComponentType<Gravity>()))
    //    {
    //        if (ImGui::Button("Gravity"))
    //        {
    //            coordinator->AddComponent<Gravity>(
    //                entity,
    //                { glm::vec3(0.0f, 0, 0.0f) });
    //        }
    //    }
    //    if (!signature.test(coordinator->GetComponentType<Player>()))
    //    {
    //        if (ImGui::Button("Player"))
    //        {

    //        }
    //    }
    //    if (!signature.test(coordinator->GetComponentType<Renderable>()))
    //    {
    //        if (ImGui::Button("Renderable"))
    //        {
    //            coordinator->AddComponent(
    //                entity,
    //                Renderable{
    //                    .color = glm::vec4(40,40,40, 1)
    //                });
    //        }
    //    }
    //    if (!signature.test(coordinator->GetComponentType<RigidBody>()))
    //    {
    //        if (ImGui::Button("RigidBody"))
    //        {
    //            coordinator->AddComponent(
    //                entity,
    //                RigidBody{
    //                    .velocity = glm::vec3(0.0f, 0.0f, 0.0f),
    //                    .acceleration = glm::vec3(0.0f, 0.0f, 0.0f)
    //                });
    //        }
    //    }

    //    /*if (!signature.test(coordinator->GetComponentType<Collision>()))
    //    {
    //        if (ImGui::Button("Collision"))
    //        {
    //            coordinator->AddComponent(
    //                entity,
    //                Collision{

    //                });
    //        }
    //    }*/
    //    /*if (!signature.test(coordinator->GetComponentType<StaticMesh>()))
    //    {
    //        if (ImGui::Button("StaticMesh"))
    //        {
    //            coordinator->AddComponent(
    //                entity,
    //                StaticMesh{

    //                });
    //        }
    //    }*/
    //    if (!signature.test(coordinator->GetComponentType<Animated>()))
    //    {
    //        if (ImGui::Button("Animated"))
    //        {
    //            coordinator->AddComponent(
    //                entity,
    //                Animated{

    //                });
    //        }
    //    }
    //}

    //if (ImGui::CollapsingHeader("Remove component"))
    //{
    //    if (signature.test(coordinator->GetComponentType<Transform>()))
    //    {
    //        if (ImGui::Button("Transform"))
    //        {
    //            coordinator->RemoveComponent<Transform>(entity);
    //        }
    //    }
    //    if (signature.test(coordinator->GetComponentType<Gravity>()))
    //    {
    //        if (ImGui::Button("Gravity"))
    //        {
    //            coordinator->RemoveComponent<Gravity>(entity);
    //        }
    //    }
    //    if (signature.test(coordinator->GetComponentType<Player>()))
    //    {
    //        if (ImGui::Button("Player"))
    //        {
    //            coordinator->RemoveComponent<Player>(entity);
    //        }
    //    }
    //    if (signature.test(coordinator->GetComponentType<Renderable>()))
    //    {
    //        if (ImGui::Button("Renderable"))
    //        {
    //            coordinator->RemoveComponent<Renderable>(entity);
    //        }
    //    }
    //    if (signature.test(coordinator->GetComponentType<RigidBody>()))
    //    {
    //        if (ImGui::Button("RigidBody"))
    //        {
    //            coordinator->RemoveComponent<RigidBody>(entity);
    //        }
    //    }
    //    /*if (signature.test(coordinator->GetComponentType<StaticMesh>()))
    //    {
    //        if (ImGui::Button("StaticMesh"))
    //        {
    //            coordinator->RemoveComponent<StaticMesh>(entity);
    //        }
    //    }*/
    //    /*if (signature.test(coordinator->GetComponentType<Collision>()))
    //    {
    //        if (ImGui::Button("Collision"))
    //        {
    //            coordinator->RemoveComponent<Collision>(entity);
    //        }
    //    }*/
    //    if (signature.test(coordinator->GetComponentType<Animated>()))
    //    {
    //        if (ImGui::Button("Animated"))
    //        {
    //            coordinator->RemoveComponent<Animated>(entity);
    //        }
    //    }
    //    if (signature.test(coordinator->GetComponentType<BehaviourScript>()))
    //    {
    //        if (ImGui::Button("BehaviourScript"))
    //        {
    //            coordinator->RemoveComponent<BehaviourScript>(entity);
    //        }
    //    }
    //}

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
                void* componentData = engineAPI->GetComponent(entity, componentInfo->name);

                // Iterate through the reflected variables of the component.
                for (auto& variable : componentInfo->variables) {
                    // Get a pointer to the variable's data using the base pointer and offset.
                    char* varDataPtr = (char*)componentData + variable.offset;

                    // Use the variable's type information to choose the correct ImGui widget.
                    if (strcmp(variable.type->name, "float") == 0) {
                        ImGui::DragFloat(variable.name, (float*)varDataPtr);
                    }
                    else if (strcmp(variable.type->name, "int") == 0) {
                        ImGui::InputInt(variable.name, (int*)varDataPtr);
                    }
                    else if (strcmp(variable.type->name, "bool") == 0) {
                        ImGui::Checkbox(variable.name, (bool*)varDataPtr);
                    }
                    // Add more conditions for other types like glm::vec3, glm::vec4, etc.
                    // For example, for a glm::vec3:
                    else if (strcmp(variable.type->name, "glm::vec3") == 0) {
                        ImGui::DragFloat3(variable.name, (float*)varDataPtr);
                    }
                }
            }
        }
    }

	ImGui::End();
}