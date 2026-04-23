# ReEngine

Custom C++20 game engine and editor developed as an engineering thesis project.

The repository contains a Windows-only runtime built around an ECS coordinator, a reflection-driven editor, a DLL-based system/plugin model, an OpenGL renderer, and PhysX-based 3D physics. The codebase is split into engine core projects and separately compiled runtime systems loaded from configuration at startup.

## What It Contains

- ECS-style runtime with `Coordinator`, component storage, entity signatures, and system registration.
- Reflection registry used for component/system discovery, editor property inspection, and scene serialization.
- Multi-threaded frame loop with separate update and render threads synchronized through a barrier.
- Wave-based system scheduler that resolves `RunAfter` and `RunBefore` dependencies.
- OpenGL renderer with deferred shading, shadow mapping, skybox rendering, instancing, static mesh rendering, and skeletal mesh support.
- Asset pipeline for importing source assets into custom runtime formats.
- Editor built with Dear ImGui and ImGuizmo.
- Hot-reload path for game DLLs through shadow-copy loading.
- Packaging step for creating a runnable build with engine binaries, system DLLs, content, and shaders.

## Architecture

```text
Game/Editor EXE
    |
    v
ReEngine.dll
    |
    +-- Coordinator
    |     +-- EntityManager
    |     +-- ComponentManager
    |     +-- SystemManager
    |     +-- EventManager
    |     +-- SceneManager
    |     +-- AssetManager
    |
    +-- Application
          +-- update thread
          +-- render thread
          +-- barrier sync
          +-- system graph execution
    |
    v
Loadable system DLLs
    +-- OpenGLRenderer.dll
    +-- Physics3D.dll
    +-- Animator.dll
    +-- StateMachineSystem.dll
    +-- other optional modules
```

### Core projects

- `ReEngine/ReEngine.sln`
  - `ReEngine`: main runtime DLL
  - `ReEngineCore`: shared engine/core utilities
  - `ReflectionCore`: runtime reflection registry
  - `ReEngineEditor`: editor executable
  - `ReEngineRunner`: packaged game executable

- `ReEngineSystems/ReEngineSystems.sln`
  - `OpenGLRenderer`
  - `Physics2D`
  - `Physics3D`
  - `Animator`
  - `StateMachineSystem`
  - `ReEngineAiAssistant`
  - `ReEngineModule1` (`WorldGeneration`)

## Runtime Model

### 1. Module loading

`ProjectBuilder` loads the game DLL and runtime systems from a JSON project file. Systems are regular DLLs exporting:

```cpp
extern "C" System* CreateSystem();
```

The builder reads reflection metadata, registers components and systems in the coordinator, and configures system signatures and graph dependencies from reflected fields such as:

- `ComponentsToRegister`
- `SystemsToRunAfter`
- `SystemsToRunBefore`
- `WriteComponents`
- `RunOnMainThread`

### 2. ECS and reflection

The engine stores components in type-indexed arrays and tracks entity membership with bitset signatures. Reflection metadata is used in three places:

- runtime registration of systems/components
- editor property rendering without hardcoded per-component UI
- JSON scene/prefab serialization and deserialization

Generated reflection sources live in `generated/*.gen.cpp` across engine and system projects.

### 3. System scheduling

`SystemGraph` builds execution waves from declared dependencies. Systems in the same wave are dispatched in parallel through the thread pool unless they declare `RunOnMainThread`.

This allows the frame to execute as:

```text
load reflected systems
    -> build dependency graph
    -> execute wave 0
    -> wait
    -> execute wave 1
    -> wait
    -> ...
```

### 4. Frame threading

`Application` owns the main runtime loop:

- update thread
  - processes input
  - gathers render commands
  - runs gameplay/simulation systems
  - processes pending entity deletion
- render thread
  - dispatches pending GPU uploads from `AssetManager`
  - renders each viewport
  - presents the frame
- barrier completion
  - swaps selected component buffers
  - advances application state transitions
  - coordinates hot reload

## Rendering

The current renderer lives in `ReEngineSystems/OpenGLRenderer`.

### Implemented pipeline

- GLFW window/context creation
- GLEW-based OpenGL function loading
- deferred G-buffer pipeline
- full-screen lighting pass
- directional shadow map pass
- skybox rendering
- instanced drawing for batched primitives
- skeletal mesh rendering with bone matrix upload
- editor viewport rendering through a framebuffer abstraction

### Material system

The engine includes a node-based material editor. Material graphs are compiled into GLSL at runtime and then built into OpenGL shader programs. This is implemented in:

- `ReEngineCore/Graph`
- `ReEngineCore/MaterialSystem`
- `ReEngineEditor/Panels/MaterialGraphPanel.cpp`

### Asset upload path

Assets are loaded on background threads and uploaded on the render thread:

1. CPU asset data is loaded asynchronously.
2. Upload lambdas are queued in `AssetManager`.
3. `DispatchUploads()` executes OpenGL upload work on the render thread.

This avoids creating GL resources from worker threads.

## Physics and Animation

### Physics

- `Physics3D` integrates NVIDIA PhysX.
- Supported collider/component types include box, sphere, capsule, mesh, and heightfield data.
- Physics writes back into ECS components such as `Transform` and `RigidBody`.

### Animation

- Skeletal meshes are imported into custom runtime formats.
- Animation clips are serialized to binary assets.
- The `Animator` system updates final bone matrices per entity.
- A separate animation graph/state machine system drives clip transitions and blackboard parameters.

## Editor

The editor is built on Dear ImGui and includes:

- scene viewport
- property inspector
- content browser
- selection/entity tools
- transform gizmos through ImGuizmo
- systems manager panel
- material graph editor
- animation graph editor
- scene settings and packaging UI

The editor runs on top of the same runtime DLL and systems used by the packaged application.

## Asset and Content Pipeline

Imported runtime asset formats used in this repository:

- `.remesh`: static mesh
- `.reskel`: skeletal mesh
- `.retex`: texture
- `.reanim` / `.rsm`: animation
- `.material`: material graph
- `.scene`: scene
- `.prefab`: prefab

Source import currently relies on Assimp and custom serializers in `ReEngineEditor/AssetManagement`.

Scenes and prefabs are JSON-based and reflection-driven. Scene loading also preloads referenced meshes, textures, materials, and animation graphs.

## Build Requirements

- Windows
- Visual Studio 2022
- MSVC toolset `v143` for most projects
- OpenGL 4.6-capable GPU/driver
- Bundled third-party libraries under `ReEngine/Libs`

Third-party dependencies visible in the project files:

- GLFW
- GLEW
- GLM
- Assimp
- NVIDIA PhysX
- Dear ImGui
- ImGuizmo
- nlohmann/json

## Build Notes

This repository is not fully path-clean yet. Some `.vcxproj` files still contain absolute machine-specific include/library paths. If you clone the repository on another machine, fix those entries first.

Known path issues currently exist in:

- `ReEngine/ReEngine/ReEngine.vcxproj`
- `ReEngine/ReEngineEditor/ReEngineEditor.vcxproj`
- `ReEngineSystems/Physics3D/Physics3D.vcxproj`

## Recommended Build Order

1. Build `ReEngine/ReEngine.sln`
2. Build `ReEngineSystems/ReEngineSystems.sln`
3. Build a game project or the included showcase project if needed

This produces:

- engine/editor/runtime binaries in `ReEngine/bin/...`
- system DLLs in `ReEngine/bin/.../Systems`

## Running

### Editor

`ReEngineEditor.exe` expects a game DLL path as the first argument:

```powershell
ReEngineEditor.exe MyGame.dll
```

The editor then loads the project configuration associated with that DLL through `ProjectBuilder`.

### Packaged game

`ReEngineRunner.exe` reads `Game.ini` from the working directory:

```ini
Config=MyGame.json
StartScene=Content/Scenes/MyScene.scene
```

The project JSON selects the renderer, physics module, and any additional systems:

```json
{
  "name": "MyGame",
  "type": "Game",
  "Renderer": "OpenGlRenderer",
  "Physics": "Physics3D",
  "ModulesToLoad": ["Animator", "StateMachineSystem"]
}
```

## Repository Layout

```text
ReEngine/
    ReEngine/                runtime DLL
    ReEngineCore/            engine core, materials, graph, utilities
    ReEngineEditor/          editor executable and tools
    ReEngineReflectionCore/  runtime reflection registry
    ReEngineRunner/          packaged game launcher
    Tools/                   project creation tooling

ReEngineSystems/
    OpenGLRenderer/          renderer system DLL
    Physics3D/               PhysX integration
    Animator/                skeletal animation system
    StateMachineSystem/      animation/state machine logic
    ReEngineModule1/         example module
```

## Technical Highlights

- Reflection-driven engine/editor integration instead of hand-written per-type registration.
- Runtime system loading through DLL boundaries rather than a monolithic executable.
- Topologically scheduled multi-threaded system execution.
- Background asset loading with render-thread GPU uploads.
- Deferred renderer with skeletal animation support and shadow mapping.
- Node-based material graph compiled into GLSL.
- JSON scene/prefab pipeline with custom binary runtime asset formats.
- Hot-reload support for the game module using shadow-copy DLL loading.

## Current Scope

This is an experimental engine built for thesis work, not a general-purpose production engine. The codebase is most useful as a systems project demonstrating engine architecture, tooling, rendering, runtime reflection, and editor/runtime integration.
