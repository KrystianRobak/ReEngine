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

    // --- Entity Management ---
    ENGINE_API uint32_t CreateEntity() {
        return Coordinator::GetCoordinator()->CreateEntity();
    }

    ENGINE_API uint32_t CreateLightEntity() {
        return Coordinator::GetCoordinator()->CreateLightEntity();
    }

    ENGINE_API void DestroyEntity(uint32_t entity) {
        Coordinator::GetCoordinator()->DestroyEntity(entity);
    }

    ENGINE_API uint32_t GetEntitiesAmount() {
        return Coordinator::GetCoordinator()->GetEntitiesAmount();
    }

    ENGINE_API uint32_t GetLightEntitiesAmount() {
        return Coordinator::GetCoordinator()->GetLightEntitiesAmount();
    }

    ENGINE_API void SetSelectedEntity(uint32_t entity) {
        Coordinator::GetCoordinator()->SetSelectedEntity(entity);
    }

    ENGINE_API uint32_t GetSelectedEntity() {
        return Coordinator::GetCoordinator()->GetSelectedEntity();
    }

    // === Component Management (NEW) ===
    ENGINE_API void Coordinator_RegisterComponent(const Reflection::ClassInfo* Class)
    {
        Coordinator::GetCoordinator()->RegisterComponent(Class);
    }

    ENGINE_API void Coordinator_AddComponent(uint32_t entity, const char* componentTypeName, void* componentData) {
        Coordinator::GetCoordinator()->AddComponent(entity, componentTypeName, componentData);
    }

    ENGINE_API void Coordinator_RemoveComponent(uint32_t entity, const char* componentTypeName) {
        Coordinator::GetCoordinator()->RemoveComponent(entity, componentTypeName);
    }

    ENGINE_API void* Coordinator_GetComponent(uint32_t entity, const char* componentTypeName) {
        return Coordinator::GetCoordinator()->GetComponent(entity, componentTypeName);
    }

    ENGINE_API bool Coordinator_HasComponent(uint32_t entity, const char* componentTypeName) {
        return Coordinator::GetCoordinator()->GetComponent(entity, componentTypeName) != nullptr;
    }

    // System methods
    ENGINE_API System* RegisterSystem(const Reflection::ClassInfo* classInfo)
    {
        return Coordinator::GetCoordinator()->RegisterSystem(classInfo);
    }

    ENGINE_API System* GetSystem(const std::string& typeName)
    {
        return Coordinator::GetCoordinator()->GetSystem(typeName);
    }


    void SetSystemSignature(const std::string& typeName, Signature signature)
    {
        Coordinator::GetCoordinator()->SetSystemSignature(typeName, signature);
    }

    // === Component Type Information (NEW & Improved) ===
    ENGINE_API int Coordinator_GetComponentTypeCount() {
        return 0;
    }

    ENGINE_API const char* Coordinator_GetComponentTypeName(int index) {
        return nullptr; // Return null for invalid index
    }

}