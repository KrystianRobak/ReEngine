#pragma once

#include "../System.h"
#include "Shader.h"
#include "Engine/Components/Camera.h"
#include <memory>

#include "Engine/Core/Coordinator/Coordinator.h"


class Event;


class RenderSystem : public System
{
public:
	void RenderBorder();
	void Init();

	void Cleanup();

	void SetupModelAndMesh(std::shared_ptr<Coordinator> &coordinator, const Entity &entity, Shader &ChangeShader);

	void OnLightEntityAdded();
	void RecompileShader();
	
	
	void Update(float dt);

	void BindCamera(Camera* CameraToUse)
	{
		CurrentlyUsedCamera = CameraToUse;
	}

private:
	void WindowSizeListener(Event& event);

	std::unique_ptr<Shader> shader;
	std::unique_ptr<Shader> LightShader;
	std::unique_ptr<Shader> AABBshader;

	Camera* CurrentlyUsedCamera;

	GLuint mVao{};
	GLuint mVboVertices{};
	GLuint mVboNormals{};

	glm::mat4 projection;
	glm::mat4 view;

};
