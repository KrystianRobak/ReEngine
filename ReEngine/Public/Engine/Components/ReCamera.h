#pragma once

#include <glm/glm.hpp>
#include "Transform.h"
#include <cmath>

#define myPi 3.14159265358979323846264338327950288

REFCOMPONENT()
struct Camera
{
	REFVARIABLE()
	Transform CameraTransform
	{
		glm::vec3(150.0f, 150.0f, 150.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
	};
	REFVARIABLE()
	glm::vec3 cameraFront = glm::vec3(-1.0f, -1.0f, -1.0f);
	
	REFVARIABLE()
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);


	bool firstMouse = true;
	
	REFVARIABLE()
	float yaw = -90.0f;

	REFVARIABLE()
	float pitch = 0.0f;

	REFVARIABLE()
	float lastX = 800.0f / 2.0;

	REFVARIABLE()
	float lastY = 600.0 / 2.0;

	REFVARIABLE()
	float fov = 45.0f;

	REFVARIABLE()
	float aspectRatio = 800.0f / 600.0f;
};

namespace ReCamera
{
	inline glm::mat4 GetViewMatrix(Camera& camera)
	{
		return glm::lookAt(camera.CameraTransform.position, camera.CameraTransform.position + camera.cameraFront, camera.cameraUp);
	}

	inline glm::mat4 GetProjectionMatrix(Camera& camera)
	{
		return glm::perspective(glm::radians(camera.fov), camera.aspectRatio, 0.1f, 1000.f);
	}

	inline glm::mat4 GetModelMatrix(const Transform& t)
	{
		glm::mat4 model = glm::translate(glm::mat4(1.0f), t.position);
		model *= glm::mat4_cast(t.rotation);
		model = glm::scale(model, t.scale);
		return model;
	}
}



