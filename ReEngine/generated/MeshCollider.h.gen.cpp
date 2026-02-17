// Auto-generated reflection file for MeshCollider.h
#include "MeshCollider.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {


struct MeshCollider_AutoRegister {
    MeshCollider_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "MeshCollider";
        ci.fullName = "MeshCollider";
        ci.module = "ReEngine";
        ci.size = sizeof(MeshCollider);
        ci.category = Reflection::TypeCategory::Struct;
        ci.isClass = false;
        ci.isStruct = true;
        ci.construct = []() -> void* { return new MeshCollider(); };
        ci.destruct = [](void* p) { delete static_cast<MeshCollider*>(p); };
        Reflection::Registry::Instance().RegisterComponent(std::move(ci));
    }
};
static MeshCollider_AutoRegister _meshcollider_autoreg;

} // namespace ReflectionGenerated

