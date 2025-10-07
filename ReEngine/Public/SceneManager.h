#pragma once

#include "ReScene.h"
#include "ReCamera.h"

class SceneManager
{
public:
	SceneManager()
	{
		currentScene_ = new ReScene("DefaultScene", new Camera());
	}

	ReScene* currentScene_;

};

