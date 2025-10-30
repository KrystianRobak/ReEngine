#pragma once

#include "imgui/imgui.h"

#include "stb/stb_image.h"
#include "glm/glm.hpp"
#include <GL/glew.h>
#include "GLFW/glfw3.h"

#include "ReflectionEngine.h"
#include "Api/EngineApi/CoordinatorEditorApi.h"
#include "ReTypes.h"
#include "Event.h"

#include <vector>
#include <iostream>
#include <string>

inline ImTextureID LoadTexture(const char* path)
{
    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 4);
    if (data)
    {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);

        return (ImTextureID)(intptr_t)tex;
    }
}

class UIComponent
{
public:
	virtual void Init(Editor::IEngineEditorApi* engineAPI)
	{
		this->engineAPI = engineAPI;

		OnInit();
	}

	virtual void OnInit() {};

	virtual void Render() = 0;

protected:
	Editor::IEngineEditorApi* engineAPI = nullptr;

};