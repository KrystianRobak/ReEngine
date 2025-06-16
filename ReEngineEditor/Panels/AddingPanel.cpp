#include "Engine/Systems/UI/Panels/AddingPanel.h"
#include "Engine/Components/Gravity.h"
#include "Engine/Components/LightSource.h"
#include "Engine/Components/Renderable.h"
#include "Engine/Components/RigidBody.h"
#include "Engine/Components/Collision.h"
#include "Engine/Components/StaticMesh.h"
#include "Engine/Components/Transform.h"
#include "Engine/Core/Coordinator/Coordinator.h"

#include <random>

#include "imgui/imgui.h"

void AddingPanel::Render()
{
    std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();

    ImGui::Begin("AddingPanel");
    if (ImGui::CollapsingHeader("Shapes"))
    {
        ImGui::Selectable("Sphere");
        if (ImGui::IsItemClicked())
        {
            int16_t entity = coordinator->CreateEntity();
            coordinator->AddComponent(entity, Transform
                {
                    .position = glm::vec3(1, 1, -30),
                    .rotation = glm::vec3(0, 0.1, 0),
                    .scale = glm::vec3(1, 1, 1)
                });

            coordinator->AddComponent(
                entity,
                StaticMesh{

                });

            coordinator->AddComponent(
                entity,
                Collision{
                });

            coordinator->AddComponent(
                entity,
                Renderable{
                    .color = glm::vec4(40,40,40, 1)
                });

            coordinator->AddComponent<Gravity>(entity, {
                glm::vec3(0.0f, 0, 0.0f)
                });

            coordinator->AddComponent(
                entity,
                RigidBody{
                    .velocity = glm::vec3(0.0f, 0.0f, 0.0f),
                    .acceleration = glm::vec3(0.0f, 0.0f, 0.0f)
                });

            auto& staticMesh = coordinator->GetComponent<StaticMesh>(entity);
            staticMesh.loadModel("model/ball/ball.obj");

            auto& renderable = coordinator->GetComponent<Renderable>(entity);

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 1.0);

            renderable.color = glm::vec3(dis(gen), dis(gen), dis(gen));
            renderable.Ambient = glm::vec3(dis(gen), dis(gen), dis(gen));
            renderable.Diffuse = glm::vec3(dis(gen), dis(gen), dis(gen));
            renderable.Specular = glm::vec3(dis(gen), dis(gen), dis(gen));
            renderable.Shininess = 50;


        }

        ImGui::Selectable("Cube");
        if (ImGui::IsItemClicked())
        {
            int16_t entity = coordinator->CreateEntity();
            coordinator->AddComponent(entity, Transform
                {
                    .position = glm::vec3(1, 1, -30),
                    .rotation = glm::vec3(0, 0.1, 0),
                    .scale = glm::vec3(3, 3, 3)
                });


            coordinator->AddComponent(
                entity,
                StaticMesh()

                );

            coordinator->AddComponent(
                entity,
                Renderable{
                    .color = glm::vec4(40,40,40, 1)
                });

            coordinator->AddComponent<Gravity>(entity, {
                glm::vec3(0.0f, 0, 0.0f)
                });

            coordinator->AddComponent(
                entity,
                RigidBody{
                    .velocity = glm::vec3(0.0f, 0.0f, 0.0f),
                    .acceleration = glm::vec3(0.0f, 0.0f, 0.0f)
                });

            auto& staticMesh = coordinator->GetComponent<StaticMesh>(entity);
            staticMesh.loadModel("model/cube/cube.obj");
            auto& renderable = coordinator->GetComponent<Renderable>(entity);

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 1.0);

            renderable.color = glm::vec3(dis(gen), dis(gen), dis(gen));
            renderable.Ambient = glm::vec3(dis(gen), dis(gen), dis(gen));
            renderable.Diffuse = glm::vec3(dis(gen), dis(gen), dis(gen));
            renderable.Specular = glm::vec3(dis(gen), dis(gen), dis(gen));
            renderable.Shininess = 50;
        }
    }
    if (ImGui::CollapsingHeader("LightSources"))
    {
        ImGui::Selectable("Light");
        if (ImGui::IsItemClicked())
        {
            int16_t entity = coordinator->CreateLightEntity();
            coordinator->AddComponent(entity, Transform
                {
                    .position = glm::vec3(1, 1, -30),
                    .rotation = glm::vec3(0, 0.1, 0),
                    .scale = glm::vec3(1, 1, 1)
                });

            coordinator->AddComponent(
                entity,
                StaticMesh{

                });

            coordinator->AddComponent(
                entity,
                LightSource{
                    .LightColor = glm::vec3(40,40,40),
                    .Ambient = glm::vec3(0.2f,0.2f,0.2f),
                    .Diffuse =  glm::vec3(0.5f, 0.5f, 0.5f),
                    .Specular = glm::vec3(1.0f, 1.0f, 1.0f)
                });

            coordinator->AddComponent(
                entity,
                RigidBody{
                    .velocity = glm::vec3(0.0f, 0.0f, 0.0f),
                    .acceleration = glm::vec3(0.0f, 0.0f, 0.0f)
                });

            coordinator->AddComponent(
                entity,
                Renderable{
                    .color = glm::vec4(40,40,40, 1)
                });

            auto& staticMesh = coordinator->GetComponent<StaticMesh>(entity);
            staticMesh.loadModel("model/ball/ball.obj");
        }
    }
           

    ImGui::End();
}
