// Auto-generated reflection file for StaticMesh.h
#include "StaticMesh.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {


struct StaticMesh_AutoRegister {
    StaticMesh_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "StaticMesh";
        ci.fullName = "StaticMesh";
        ci.module = "/Script/GeneratedModule";
        ci.size = sizeof(StaticMesh);
        ci.category = Reflection::TypeCategory::Struct;
        ci.isClass = false;
        ci.isStruct = true;
        ci.construct = []() -> void* { return new StaticMesh(); };
        ci.destruct = [](void* p) { delete static_cast<StaticMesh*>(p); };
        Reflection::Registry::Instance().RegisterComponent(std::move(ci));
    }
};
static StaticMesh_AutoRegister _staticmesh_autoreg;

} // namespace ReflectionGenerated

