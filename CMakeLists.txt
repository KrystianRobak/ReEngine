cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE "C:/Users/ragberr/.vcpkg-clion/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Project name and language
project(untitled LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Define configurations
# set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "Available build configurations" FORCE)

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/imgui
        ${CMAKE_SOURCE_DIR}/stb
        ${CMAKE_SOURCE_DIR}/Public
        ${CMAKE_SOURCE_DIR}/Private
        ${CMAKE_SOURCE_DIR}/Libs
)


# Source files including imgui sources
set(SOURCES
        Private/Engine/Systems/UI/Panels/AddingPanel.cpp
        Private/Engine/Core/Application.cpp
        Private/Engine/Systems/Input/CameraControlSystem.cpp
        Private/Engine/Components/Collision.cpp
        Private/Engine/Core/Coordinator/ComponentArray.cpp
        Private/Engine/Core/Coordinator/ComponentManager.cpp
        Private/Engine/Systems/UI/Panels/ControlPanel.cpp
        Private/Engine/Core/Coordinator/Coordinator.cpp
        Private/Engine/Core/Coordinator/EntityManager.cpp
        Private/Engine/Core/Coordinator/Event.cpp
        Private/Engine/Core/Coordinator/EventManager.cpp
        Private/Engine/Systems/UI/Panels/FileBrowser.cpp
        Private/Engine/Core/FrameBuffer.cpp
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_impl_glfw.cpp
        imgui/imgui_impl_opengl3.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        Private/Engine/Systems/UI/Panels/ItemsSelectionPanel.cpp
        Private/Engine/Components/Mesh.cpp
        Private/Engine/Components/StaticMesh.cpp
        Private/Engine/Core/Context/OpenGlContext.cpp
        Private/Engine/Core/Context/OpenGlFrameBuffer.cpp
        ParticleOpenGl.cpp
        Private/Engine/Systems/Physics/PhysicsSystem.cpp
        Private/Engine/Systems/Input/PlayerControlSystem.cpp
        Private/Engine/Systems/UI/Panels/PropertyPanel.cpp
        Private/Engine/Core/Context/RenderContext.cpp
        Private/Engine/Systems/Render/RenderSystem.cpp
        Private/Engine/Systems/UI/Panels/SceneView.cpp
        Private/Engine/Systems/Render/Shader.cpp
        Private/Engine/Systems/System.cpp
        Private/Engine/Core/Coordinator/SystemManager.cpp
        Private/Engine/Core/Context/UIContext.cpp
        Private/Engine/Systems/UI/UiSystem.cpp
        Private/Engine/Core/Window.cpp
        Private/Engine/Core/WindowManager.cpp
        Private/Engine/Components/Collision/AABB.cpp
        Private/Engine/Components/Collision/Octree.cpp
        Private/Engine/Core/Reflection/ReflectionTest.cpp
        Private/Engine/Core/Reflection/ReflectionRegistry.cpp
        Private/Engine/Systems/Animation/AnimationSystem.cpp
        Private/Engine/Systems/UI/Panels/AnimationPanel.cpp
        Private/Engine/Systems/Animation/AnimationSequence.cpp
        Private/Engine/Systems/UI/Panels/KeyframeEditorPanel.cpp
)

# Header files including imgui headers
set(HEADERS
        Public/Engine/Systems/UI/Panels/AddingPanel.h
        Public/Engine/Core/Application.h
        Public/Engine/Components/BasicInfo.h
        Public/Engine/Components/Camera.h
        Public/Engine/Systems/Input/CameraControlSystem.h
        Public/Engine/Components/Collision.h
        Public/Engine/Core/Coordinator/ComponentArray.h
        Public/Engine/Core/Coordinator/ComponentManager.h
        Public/Engine/Systems/UI/Panels/ControlPanel.h
        Public/Engine/Core/Coordinator/Coordinator.h
        Public/Engine/Core/Coordinator/EntityManager.h
        Public/Engine/Core/Coordinator/Event.h
        Public/Engine/Core/Coordinator/EventManager.h
        Public/Engine/Systems/UI/Panels/FileBrowser.h
        Public/Engine/Core/FrameBuffer.h
        GJK.h
        Public/Engine/Components/Gravity.h
        imgui/imconfig.h
        imgui/imgui.h
        imgui/imgui_impl_glfw.h
        imgui/imgui_impl_opengl3.h
        imgui/imgui_impl_opengl3_loader.h
        imgui/imgui_internal.h
        imgui/imstb_rectpack.h
        imgui/imstb_textedit.h
        imgui/imstb_truetype.h
        Public/Engine/Systems/UI/Panels/ItemsSelectionPanel.h
        Public/Engine/Core/IWindow.h
        Public/Engine/Components/LightSource.h
        Public/Engine/Components/Mesh.h
        Public/Engine/Components/StaticMesh.h
        Public/Engine/Core/Context/OpenGlContext.h
        Public/Engine/Core/Context/OpenGlFrameBuffer.h
        Public/Engine/Systems/Physics/PhysicsSystem.h
        Public/Engine/Systems/Input/PlayerControlSystem.h
        Public/Engine/Systems/UI/Panels/PropertyPanel.h
        Public/Engine/Components/Renderable.h
        Public/Engine/Core/Context/RenderContext.h
        Public/Engine/Systems/Render/RenderSystem.h
        Public/Engine/Components/RigidBody.h
        Public/Engine/Systems/UI/Panels/SceneView.h
        Public/Engine/Systems/Render/Shader.h
        Public/Engine/Systems/System.h
        Public/Engine/Core/Coordinator/SystemManager.h
        Public/Engine/Components/Thrust.h
        Public/Engine/Components/Transform.h
        Public/Engine/Core/Types.h
        Public/Engine/Systems/UI/Panels/UIComponent.h
        Public/Engine/Core/Context/UIContext.h
        Public/Engine/Systems/UI/UiSystem.h
        Public/Engine/Core/Window.h
        Public/Engine/Core/WindowManager.h
        Public/Engine/Components/Collision/AABB.h
        Public/Engine/Components/Collision/Octree.h
        Public/Engine/Core/Reflection/ReflectionMacro.h
        Public/Engine/Core/Reflection/ReflectionTest.h
        Public/Engine/Core/Reflection/ReflectionRegistry.h
        Public/Engine/Systems/UI/Panels/AnimationPanel.h
        Public/Engine/Systems/Animation/AnimationSystem.h
        Public/Engine/Core/AnimationTypes.h
        Public/Engine/Components/Animated.h
        Public/Engine/Systems/Animation/AnimationSequence.h
        Public/Engine/Systems/UI/Panels/KeyframeEditorPanel.h
)

set(glfw3_DIR "C:/Users/ragberr/.vcpkg-clion/vcpkg/installed/x64-mingw-dynamic/share/glfw3")

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries using vcpkg
find_package(glfw3 CONFIG REQUIRED )
find_package(glew CONFIG REQUIRED )
find_package(assimp REQUIRED )
find_package(glm CONFIG REQUIRED)
find_package(X11 REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        GLEW::GLEW
        glm::glm
        assimp::assimp
        opengl32
        ${X11_LIBRARIES}
        Threads::Threads
)

# Set output directories for different configurations
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
)
