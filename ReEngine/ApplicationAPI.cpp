#include "ApplicationAPI.h"

#include "Public/Engine/Core/Application.h"

extern "C" {

    ENGINE_API void* CreateApplication() {
        return static_cast<void*>(new Application());
    }

    ENGINE_API void DestroyApplication(void* app) {
        delete static_cast<Application*>(app);
    }

    ENGINE_API void Application_Init(void* app) {
        static_cast<Application*>(app)->Init();
    }

    ENGINE_API void Application_Update(void* app) {
        static_cast<Application*>(app)->Update();
    }

    ENGINE_API void Application_Render(void* app) {
        static_cast<Application*>(app)->Render();
    }

    ENGINE_API bool Application_IsRunning(void* app) {
        return static_cast<Application*>(app)->IsRunning();
    }

    // === Entity Management ===
    ENGINE_API uint32_t Coordinator_CreateEntity() {
        return Coordinator::GetCoordinator()->CreateEntity();
    }

    ENGINE_API uint32_t Coordinator_CreateLightEntity() {
        return Coordinator::GetCoordinator()->CreateLightEntity();
    }

    ENGINE_API void Coordinator_DestroyEntity(uint32_t entity) {
        Coordinator::GetCoordinator()->DestroyEntity(entity);
    }

    ENGINE_API uint32_t Coordinator_GetEntitiesAmount() {
        return Coordinator::GetCoordinator()->GetEntitiesAmount();
    }

    ENGINE_API uint32_t Coordinator_GetLightEntitiesAmount() {
        return Coordinator::GetCoordinator()->GetLightEntitiesAmount();
    }

    // === Selected Entity ===
    ENGINE_API void Coordinator_SetSelectedEntity(uint32_t entity) {
        Coordinator::GetCoordinator()->SetSelectedEntity(entity);
    }

    ENGINE_API uint32_t Coordinator_GetSelectedEntity() {
        return Coordinator::GetCoordinator()->GetSelectedEntity();
    }

    // === Camera Access ===
    ENGINE_API Camera* Coordinator_GetMainCamera() {
        return Coordinator::GetCoordinator()->GetCamera();
    }

    // === Components ===
    ENGINE_API void Coordinator_GetComponentTypes() {
        auto types = Coordinator::GetCoordinator()->GetComponentsTypes();
        // You would pass these to your reflection system
        // Example: For each pair, store or serialize {name, type}
    }

    // === Animation Scene Access ===
    ENGINE_API AnimationSequencer* Coordinator_GetScene() {
        return Coordinator::GetCoordinator()->GetScene();
    }

}