#pragma once

#include <bitset>
#include <cstdint>
#include <functional>
#include <string_view>


using Entity = std::uint32_t;

inline constexpr Entity APPLICATION = static_cast<Entity>(-1);
inline constexpr Entity GUI = static_cast<Entity>(-2);
inline constexpr Entity WINDOW = static_cast<Entity>(-3);

inline constexpr Entity MAX_ENTITIES = 110;
inline constexpr Entity MAX_LIGHT_ENTITIES = 10;
inline constexpr Entity MAX_OBJECT_ENTITIES = 100;

using ComponentType = std::uint8_t;
inline constexpr ComponentType MAX_COMPONENTS = 32;

using Signature = std::bitset<MAX_COMPONENTS>;

inline constexpr unsigned int SCR_WIDTH = 800;
inline constexpr unsigned int SCR_HEIGHT = 600;

inline constexpr std::string_view MAIN_WINDOW_NAME = "ReEngineEditor";



enum MenuType
{
	BaseMenu,
	AnimationMenu
};

static constexpr const char* EVENT_COLLISION_BEGIN = "COLLISION_BEGIN";
static constexpr const char* EVENT_COLLISION_END = "COLLISION_END";
static constexpr const char* EVENT_TRIGGER_BEGIN = "TRIGGER_BEGIN";
static constexpr const char* EVENT_TRIGGER_END = "TRIGGER_END";

struct CollisionEventPayload {
    Entity entityA;
    Entity entityB;
};

using EventType = std::string_view;
using ParamId = std::string_view;

using FunctionDelegate = std::function<void()>;

#define METHOD_LISTENER_NO_PARAM(EventType, Listener) EventType, std::bind(&Listener, this)
#define METHOD_LISTENER_ONE_PARAM(EventType, Listener) EventType, std::bind(&Listener, this, std::placeholders::_1)
#define METHOD_LISTENER_TWO_PARAM(EventType, Listener) EventType, std::bind(&Listener, this, std::placeholders::_1, std::placeholders::_2)
#define METHOD_LISTENER_THREE_PARAM(EventType, Listener) EventType, std::bind(&Listener, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
#define METHOD_LISTENER_FOUR_PARAM(EventType, Listener) EventType, std::bind(&Listener, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)

#define FUNCTION_LISTENER_ONE_PARAM(EventType, Listener) EventType, std::bind(&Listener, std::placeholders::_1)


namespace Events::Window {
    constexpr inline EventType QUIT = "Events::Window::QUIT";
    constexpr inline EventType RESIZED = "Events::Window::RESIZED";
    constexpr inline EventType INPUT = "Events::Window::INPUT";
	constexpr inline EventType FILE_DROPPED = "Events::Window::FILE_DROPPED";
}

namespace Events::Window::Input {
    constexpr inline ParamId INPUT = "Events::Window::Input::INPUT";
}

namespace Events::Window::Resized {
    constexpr inline ParamId WIDTH = "Events::Window::Resized::WIDTH";
    constexpr inline ParamId HEIGHT = "Events::Window::Resized::HEIGHT";
}

namespace Events::Application {
    constexpr inline EventType TOGGLE = "Events::Application::TOGGLE";
    constexpr inline EventType RECOMPILE_SHADER = "Events::Application::RECOMPILE_SHADER";
    constexpr inline EventType LIGHT_ENTITY_ADDED = "Events::Application::LIGHT_ENTITY_ADDED";
    constexpr inline EventType MENU_CHANGED = "Events::Application::MENU_CHANGED";
    constexpr inline EventType START_GAME = "Events::Application::START_GAME";
    constexpr inline EventType CAMERA_CHANGED = "Events::Application::CAMERA_CHANGED";
    constexpr inline EventType RECOMPILE_READY = "Events::Application::RECOMPILE_READY";
}

namespace Events::Engine::SceneManager {
    constexpr inline EventType SCENE_LOADED = "Events::Engine::SCENE_LOADED";
    constexpr inline EventType SCENE_SAVED = "Events::Engine::SCENE_SAVED";
    constexpr inline EventType SCENE_PLAY = "Events::Engine::SCENE_PLAY";
    constexpr inline EventType SCENE_STOP = "Events::Engine::SCENE_STOP";
}

namespace Events::Engine::Renderer {
    constexpr inline EventType RENDER_FINISHED = "Events::Engine::Renderer::RENDER_FINISHED";
}

namespace Events::Engine::LayerManager {
    constexpr inline EventType INITIALIZED = "Events::Engine::LayerManager::Initialized";
}

namespace Events::Editor::Gizmo {
    constexpr inline EventType TRANSLATE = "Events::Editor::Gizmo::TRANSLATE";
    constexpr inline EventType ROTATE = "Events::Editor::Gizmo::ROTATE";
    constexpr inline EventType SCALE = "Events::Editor::Gizmo::SCALE";
}

namespace Events::Editor::MaterialSystem {
    constexpr inline EventType CREATE_MATERIAL_FILE = "Events::Editor::MaterialSystem::CREATE_MATERIAL_FILE";
    constexpr inline EventType OPEN_MATERIAL_FILE = "Events::Editor::MaterialSystem::OPEN_MATERIAL_FILE";
}

namespace Events::Editor::StateMachineGraph {
    constexpr inline EventType CREATE_STATEMACHINE_FILE = "Events::Editor::StateMachineGraph::CREATE_STATEMACHINE_FILE";
    constexpr inline EventType OPEN_STATEMACHINE_FILE = "Events::Editor::StateMachineGraph::OPEN_STATEMACHINE_FILE";
}

namespace Events::Editor::FileBrowser {
    constexpr inline EventType LOCATE_FILE = "Events::Editor::FileBrowser::LOCATE_FILE";
    constexpr inline EventType SAVE_PREFAB = "Events::Editor::FileBrowser::SAVE_PREFAB";
}

namespace Events::Physics {
    constexpr inline EventType COLLISION_HAPPENED = "Events::Physics::COLLISION_HPPENED";
}
