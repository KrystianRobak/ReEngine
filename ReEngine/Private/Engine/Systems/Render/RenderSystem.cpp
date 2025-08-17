#include "Engine/Systems/Render/RenderSystem.h"

#include "Engine/Components/Renderable.h"
#include "Engine/Components/Transform.h"
#include "Engine/Core/Coordinator/Coordinator.h"
#include "Engine/Systems/Render/Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#include "Engine/Components/StaticMesh.h"
#include "Engine/Components/Collision.h"
#include "Engine/Components/LightSource.h"

void RenderSystem::OnLightEntityAdded() 
{
	/*std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();

	shader->ChangeShaderDefineStatus(coordinator->GetLightEntitiesAmount());
	RecompileShader();*/
}

void RenderSystem::RecompileShader()
{
	shader = std::make_unique<Shader>("shaders/Default.vs", "shaders/Default.fs");
	LightShader = std::make_unique<Shader>("shaders/LightSourceShader/LightSource.vs", "shaders/LightSourceShader/LightSource.fs");
}

void RenderSystem::Init()
{
	//std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();

	//coordinator->AddEventListener(METHOD_LISTENER_ONE_PARAM(Events::Window::RESIZED, RenderSystem::WindowSizeListener));
	//coordinator->AddEventListener(METHOD_LISTENER_NO_PARAM(Events::Application::RECOMPILE_SHADER, RenderSystem::RecompileShader));
	//
	//coordinator->AddEventListener(METHOD_LISTENER_NO_PARAM(Events::Application::LIGHT_ENTITY_ADDED, RenderSystem::OnLightEntityAdded));
	//
	//shader = std::make_unique<Shader>("shaders/Default.vs", "shaders/Default.fs");
	//LightShader = std::make_unique<Shader>("shaders/LightSourceShader/LightSource.vs", "shaders/LightSourceShader/LightSource.fs");
	//AABBshader = std::make_unique<Shader>("shaders/AABB/AABB.vs", "shaders/AABB/AABB.fs");

	//std::cout << "Initializing AABB system..." << std::endl;
	//AABB::buffersInitialized = false; // Reset flag to ensure initialization happens

	//AABB::setupStaticBuffers();

	//

	//BindCamera(coordinator->GetCamera());
}

void RenderSystem::Update(float dt)
{
	/*std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();

	std::shared_ptr<ComponentManager> Manager = coordinator->GetComponentManager();

	std::shared_ptr<ComponentArray<Renderable>> RenderableArray = Manager->GetComponentArray<Renderable>();
	std::shared_ptr<ComponentArray<Transform>> TransformArray = Manager->GetComponentArray<Transform>();
	std::shared_ptr<ComponentArray<LightSource>> LightSourceArray = Manager->GetComponentArray<LightSource>();

	std::vector<Entity> entitiesUsingShader;
	std::vector<Entity> entitiesUsingLightShader;

	for (auto const& entity : mEntities)
	{
		if (entity >= 11)
			entitiesUsingShader.push_back(entity);
		else
			entitiesUsingLightShader.push_back(entity);
	}

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glBindVertexArray(mVao);

	projection = glm::perspective(glm::radians(CurrentlyUsedCamera->fov), 800.f/600.f, 0.1f, 100.0f);
	view = glm::lookAt(
		CurrentlyUsedCamera->CameraTransform.position,
		CurrentlyUsedCamera->CameraTransform.position + CurrentlyUsedCamera->cameraFront,
		CurrentlyUsedCamera->cameraUp);

	shader->Use();

	for(int i = 0; i < entitiesUsingLightShader.size(); i++)
	{
		auto LightData = LightSourceArray->GetData(entitiesUsingLightShader[i]);
		auto PositionData = TransformArray->GetData(entitiesUsingLightShader[i]);

		shader->SetVec3("LightArray[" + std::to_string(i) + "].Position", PositionData.position);
		shader->SetVec3("LightArray[" + std::to_string(i) + "].Ambient", LightData.Ambient);
		shader->SetVec3("LightArray[" + std::to_string(i) + "].Diffuse", LightData.Diffuse);
		shader->SetVec3("LightArray[" + std::to_string(i) + "].Specular", LightData.Specular);
	}

	shader->SetMat4("Projection", projection);
	shader->SetMat4("View", view);
	shader->SetVec3("ViewPos", CurrentlyUsedCamera->CameraTransform.position);

	for (auto const& entity : entitiesUsingShader)
	{
		auto const& renderable = coordinator->GetComponent<Renderable>(entity);

		shader->SetVec3("ObjectColor", renderable.color);

		shader->SetVec3("Material.Ambient", renderable.Ambient);
		shader->SetVec3("Material.Diffuse", renderable.Diffuse);
		shader->SetVec3("Material.Specular", renderable.Specular);
		shader->SetFloat("Material.Shininess", renderable.Shininess);

		SetupModelAndMesh(coordinator, entity, *shader.get());
	}


	LightShader->Use();

	LightShader->SetMat4("Projection", projection);
	LightShader->SetMat4("View", view);

	for (auto const& entity : entitiesUsingLightShader)
	{
		SetupModelAndMesh(coordinator, entity, *LightShader.get());
	}

	AABBshader->Use();

	AABBshader->SetMat4("Projection", projection);
	AABBshader->SetMat4("View", view);

	for (auto const& entity : mEntities) {
		auto signature = coordinator->GetEntitySignature(entity);
		if (signature.test(coordinator->GetComponentType<Collision>()) &&
			signature.test(coordinator->GetComponentType<StaticMesh>()) &&
			signature.test(coordinator->GetComponentType<Transform>())) {

			auto& staticMesh = coordinator->GetComponent<StaticMesh>(entity);
			auto& collision = coordinator->GetComponent<Collision>(entity);
			auto& transform = coordinator->GetComponent<Transform>(entity);

			auto meshBounds = staticMesh.getAABB();

			collision.boundingBox.updatePosition(transform.position, transform.rotation, transform.scale);
			collision.boundingBox.SetOffsets(meshBounds);
			collision.boundingBox.draw(AABBshader.get());
		}
	}

	glBindVertexArray(0);*/
}

void RenderSystem::Cleanup()
{

}



void RenderSystem::SetupModelAndMesh(std::shared_ptr<Coordinator>& coordinator, const Entity& entity, Shader& ChangeShader)
{
	/*auto signature = coordinator->GetEntitySignature(entity);

	auto const& transform = coordinator->GetComponent<Transform>(entity);
	
	glm::mat4 model = glm::translate(glm::mat4(1.0f), transform.position) *
				  glm::mat4_cast(transform.rotation) *
				  glm::scale(glm::mat4(1.0f), transform.scale);


	ChangeShader.SetMat4("Model", model);

	if (signature.test(coordinator->GetComponentType<StaticMesh>()))
	{
		auto& staticMesh = coordinator->GetComponent<StaticMesh>(entity);
		staticMesh.Draw(ChangeShader);
	}*/
}


void RenderSystem::WindowSizeListener(Event& event)
{
	//std::shared_ptr<Coordinator> coordinator = Coordinator::GetCoordinator();

	//auto windowWidth = event.GetParam<unsigned int>(Events::Window::Resized::WIDTH);
	//auto windowHeight = event.GetParam<unsigned int>(Events::Window::Resized::HEIGHT);

	//Camera* camera = coordinator->GetCamera();
	////camera.projectionTransform = Camera::MakeProjectionTransform(45.0f, 0.1f, 1000.0f, windowWidth, windowHeight);
	//

}

