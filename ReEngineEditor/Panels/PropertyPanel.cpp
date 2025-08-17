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
    //std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();

    //if (ImGui::CollapsingHeader("Add component"))
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


void PropertyPanel::Render()
{
    //std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();
    //std::int32_t entity = coordinator->GetSelectedEntity();
    //if (entity < 0 && entity > MAX_ENTITIES)
    //    return;

    //std::bitset<32> signature = coordinator->GetEntitySignature(entity);
    //ImGui::Begin("Properties");
    //RenderComponentsMenu(entity, signature);
    //signature = coordinator->GetEntitySignature(entity);
   

    //std::string label = "Components: ##" + std::to_string(entity);

    //if (signature.test(coordinator->GetComponentType<Transform>()))
    //{
    //    //coordinator->GetComponent<Transform>(entity).GenerateGUIElements(entity);
    //}

    //if (signature.test(coordinator->GetComponentType<Renderable>()))
    //{
    //    //coordinator->GetComponent<Renderable>(entity).GenerateGUIElements(entity);
    //}

    //if (signature.test(coordinator->GetComponentType<Gravity>()))
    //{
    //   // coordinator->GetComponent<Gravity>(entity).GenerateGUIElements(entity);
    //}

    //if (signature.test(coordinator->GetComponentType<RigidBody>()))
    //{
    //   // coordinator->GetComponent<RigidBody>(entity).GenerateGUIElements(entity);
    //}

    //if (signature.test(coordinator->GetComponentType<LightSource>()))
    //{
    //   // coordinator->GetComponent<LightSource>(entity).GenerateGUIElements(entity);
    //}
    //
    //if (signature.test(coordinator->GetComponentType<Animated>()))
    //{
    //    //coordinator->GetComponent<Animated>(entity);
    //}

    //if (signature.test(coordinator->GetComponentType<BehaviourScript>()))
    //{
    //    //coordinator->GetComponent<BehaviourScript>(entity).GenerateGUIElements(entity);
    //}

    //ImGui::Text(label.c_str());
    //if (ImGui::BeginDragDropTarget())
    //{
    //    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DIRECTORY_ENTRY"))
    //    {
    //        if (payload->DataSize > 0) 
    //        {
    //            std::string nameWithPath(static_cast<const char*>(payload->Data));
    //            std::vector<std::string> parts = splitString(nameWithPath, '|');
    //            if (parts.size() == 2) 
    //            {
    //                std::string name = parts[0];
    //                std::string path = parts[1];
    //                //auto& staticMesh = coordinator->GetComponent<StaticMesh>(entity);
    //                //staticMesh.loadModel(path);
    //            }
    //        }
    //    }
    //ImGui::EndDragDropTarget();
    //}

    //ImGui::End();
}