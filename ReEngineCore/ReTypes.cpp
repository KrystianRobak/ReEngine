#include "ReTypes.h"


namespace Events::Window {
	const EventType QUIT = "Events::Window::QUIT";
	const EventType RESIZED = "Events::Window::RESIZED";
	const EventType INPUT = "Events::Window::INPUT";
}

namespace Events::Window::Input {
	const ParamId INPUT = "Events::Window::Input::INPUT";
}

namespace Events::Window::Resized {
	const ParamId WIDTH = "Events::Window::Resized::WIDTH";
	const ParamId HEIGHT = "Events::Window::Resized::HEIGHT";
}

namespace Events::Application {
	const EventType TOGGLE = "Events::Application::TOGGLE";
	const EventType RECOMPILE_SHADER = "Events::Application::RECOMPILE_SHADER";
	const EventType LIGHT_ENTITY_ADDED = "Events::Application::LIGHT_ENTITY_ADDED";
	const EventType MENU_CHANGED = "Events::Application::MENU_CHANGED";
	const EventType START_GAME = "Events::Application::START_GAME";
}

namespace Events::Engine::Renderer {
	const EventType RENDER_FINISHED = "Events::Engine::Renderer::RENDER_FINISHED";
}

namespace Events::Engine::LayerManager {
	const EventType INITIALIZED = "Events::Engine::LayerManager::Initialized";
}