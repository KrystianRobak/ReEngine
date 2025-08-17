// Auto-generated reflection file for PhysicsSystem.h
#include "PhysicsSystem.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {

static Reflection::FunctionPtr PhysicsSystem_Init_Hook = nullptr;
static Reflection::FunctionPtr PhysicsSystem_Update_Hook = nullptr;

static void PhysicsSystem_Init_Invoke(void* instance, void** args, void* ret) {
    if (PhysicsSystem_Init_Hook) {
        PhysicsSystem_Init_Hook(instance, args, ret);
        return;
    }
    auto* obj = static_cast<PhysicsSystem*>(instance);
    obj->Init();
}

static void PhysicsSystem_Update_Invoke(void* instance, void** args, void* ret) {
    if (PhysicsSystem_Update_Hook) {
        PhysicsSystem_Update_Hook(instance, args, ret);
        return;
    }
    auto* obj = static_cast<PhysicsSystem*>(instance);
    obj->Update(*static_cast<float*>(args[0]));
}

static std::vector<Reflection::ReflectedFunction> PhysicsSystem_Functions;
static std::vector<Reflection::BaseClassInfo> PhysicsSystem_Bases;
struct PhysicsSystem_AutoRegister {
    PhysicsSystem_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "PhysicsSystem";
        ci.fullName = "PhysicsSystem";
        ci.module = "/Script/GeneratedModule";
        ci.size = sizeof(PhysicsSystem);
        ci.category = Reflection::TypeCategory::Class;
        ci.isClass = true;
        ci.isStruct = false;
        ci.construct = []() -> void* { return new PhysicsSystem(); };
        ci.destruct = [](void* p) { delete static_cast<PhysicsSystem*>(p); };
        PhysicsSystem_Bases.clear();
        if (auto* __base = Reflection::Registry::Instance().FindClass("System"))
            PhysicsSystem_Bases.push_back(Reflection::BaseClassInfo{ __base, (reinterpret_cast<std::size_t>(static_cast<System*>(reinterpret_cast<PhysicsSystem*>(1))) - 1) });
        ci.bases = PhysicsSystem_Bases;
        PhysicsSystem_Functions.clear();
        {
            auto* retType = Reflection::Registry::Instance().GetOrCreateType("void");
            std::vector<const Reflection::TypeInfo*> paramTypes;
            Reflection::ReflectedFunction rf = {
                "Init", "public",
                false, true, false,
                retType,
                paramTypes,
                &PhysicsSystem_Init_Invoke
            };
            PhysicsSystem_Functions.push_back(std::move(rf));
        }
        {
            auto* retType = Reflection::Registry::Instance().GetOrCreateType("void");
            std::vector<const Reflection::TypeInfo*> paramTypes;
            paramTypes.push_back(Reflection::Registry::Instance().GetOrCreateType("float"));
            Reflection::ReflectedFunction rf = {
                "Update", "public",
                false, true, false,
                retType,
                paramTypes,
                &PhysicsSystem_Update_Invoke
            };
            PhysicsSystem_Functions.push_back(std::move(rf));
        }
        ci.functions = PhysicsSystem_Functions;
        Reflection::Registry::Instance().RegisterSystem(std::move(ci));
    }
};
static PhysicsSystem_AutoRegister _physicssystem_autoreg;

} // namespace ReflectionGenerated

