#include "Engine/Core/WindowManager.h"

#include "Engine/Core/Coordinator/Coordinator.h"
#include <bitset>
#include <iostream>


// TODO: Return error to caller
void WindowManager::Init(
	std::string const& windowTitle, unsigned int windowWidth, unsigned int windowHeight, unsigned int windowPositionX,
	unsigned int windowPositionY)
{
	glewExperimental = true; // Needed for core profile
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
	}

	// Create a windowed mode window and its OpenGL context
	
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	mWindow = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), NULL, NULL);
	if (!mWindow) {
		glfwTerminate();
	}

	// Make the window's context current
	glfwMakeContextCurrent(mWindow);
	glewExperimental = true;
	glfwSetInputMode(mWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	// Initialize GLEW

	if (glewInit() != GLEW_OK) {
	}

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 255.0f, 255.4f, 0.0f);
}


void WindowManager::Update()
{
	glfwSwapBuffers(mWindow);
}


void WindowManager::Shutdown()
{
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

//void WindowManager::ProcessEvents()
//{
//	glfwPollEvents();
//
//	bool buttonStateChanged = true;
//
//	if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE))
//	{
//		coordinator->SendEvent(Events::Window::QUIT);
//	}
//	else if (glfwGetKey(mWindow, GLFW_KEY_W))
//	{
//		mButtons.set(static_cast<std::size_t>(InputButtons::W));
//	}
//	else if (glfwGetKey(mWindow, GLFW_KEY_A))
//	{
//		mButtons.set(static_cast<std::size_t>(InputButtons::A));
//	}
//	else if (glfwGetKey(mWindow, GLFW_KEY_S))
//	{
//		mButtons.set(static_cast<std::size_t>(InputButtons::S));
//	}
//	else if (glfwGetKey(mWindow, GLFW_KEY_D))
//	{
//		mButtons.set(static_cast<std::size_t>(InputButtons::D));
//	}
//	else if (glfwGetKey(mWindow, GLFW_KEY_Q))
//	{
//		mButtons.set(static_cast<std::size_t>(InputButtons::Q));
//	}
//	else if (glfwGetKey(mWindow, GLFW_KEY_E))
//	{
//		mButtons.set(static_cast<std::size_t>(InputButtons::E));
//	}
//	else
//	{
//		buttonStateChanged = false;
//	}
//
//	if (buttonStateChanged)
//	{
//		Event event(Events::Window::INPUT);
//		event.SetParam(Events::Window::Input::INPUT, mButtons);
//		coordinator->SendEvent(event);
//	}
//}