#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "stb/stb_image.h"

#include <GL/glew.h>
#include "GLFW/glfw3.h"

#include <vector>
#include <iostream>
#include <string>

class UIComponent
{
public:
	virtual void Render() = 0;
};