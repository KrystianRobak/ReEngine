#pragma once

#include "EngineApi/CoordinatorEditorApi.h"
#include "ReflectionMacros.h"
#include <ReTypes.h>
#include <set>
#include <memory>

#include "GL/glew.h"
#include "GLFW/glfw3.h"


class Commander;

class System
{
public:
	virtual void InitApi(Editor::IEngineEditorApi* engine, GLFWwindow* context)
	{
		engine_ = engine;
		renderContext = context;
	};

	virtual void Update(float dt) = 0;

	void InjectCommander(std::shared_ptr<Commander> commander)
	{
		commander_ = commander;
	}

	std::set<Entity>& GetEntities()
	{
		return this->mEntities;
	}

	GLFWwindow* GetRenderContext() { return renderContext; }

protected:
	std::set<Entity> mEntities;
	Editor::IEngineEditorApi* engine_;
	std::shared_ptr<Commander> commander_;
	GLFWwindow* renderContext = nullptr; // context for render thread
};
