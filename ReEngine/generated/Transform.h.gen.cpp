// Auto-generated reflection file for Transform.h
#include "Transform.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {


static std::vector<Reflection::ReflectedVariable> Transform_Variables;
struct Transform_AutoRegister {
    Transform_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "Transform";
        ci.fullName = "Transform";
        ci.module = "/Script/GeneratedModule";
        ci.size = sizeof(Transform);
        ci.category = Reflection::TypeCategory::Struct;
        ci.isClass = false;
        ci.isStruct = true;
        ci.construct = []() -> void* { return new Transform(); };
        ci.destruct = [](void* p) { delete static_cast<Transform*>(p); };
        Transform_Variables.clear();
        auto* vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec3");
        {
            Reflection::ReflectedVariable rv = {
                "position", "public",
                false,
                offsetof(Transform, position),
                vType
            };
            Transform_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::quat");
        {
            Reflection::ReflectedVariable rv = {
                "rotation", "public",
                false,
                offsetof(Transform, rotation),
                vType
            };
            Transform_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec3");
        {
            Reflection::ReflectedVariable rv = {
                "scale", "public",
                false,
                offsetof(Transform, scale),
                vType
            };
            Transform_Variables.push_back(std::move(rv));
        }
        ci.variables = Transform_Variables;
        Reflection::Registry::Instance().RegisterClass(std::move(ci));
    }
};
static Transform_AutoRegister _transform_autoreg;

} // namespace ReflectionGenerated

