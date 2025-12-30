// Auto-generated reflection file for SphereCollider.h
#include "SphereCollider.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {


static std::vector<Reflection::ReflectedVariable> SphereCollider_Variables;
struct SphereCollider_AutoRegister {
    SphereCollider_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "SphereCollider";
        ci.fullName = "SphereCollider";
        ci.module = "/Script/GeneratedModule";
        ci.size = sizeof(SphereCollider);
        ci.category = Reflection::TypeCategory::Struct;
        ci.isClass = false;
        ci.isStruct = true;
        ci.construct = []() -> void* { return new SphereCollider(); };
        ci.destruct = [](void* p) { delete static_cast<SphereCollider*>(p); };
        SphereCollider_Variables.clear();
        auto* vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "radius", "public",
                false,
                offsetof(SphereCollider, radius),
                vType,
                "0.5f"
            };
            SphereCollider_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "centerOffset", "public",
                false,
                offsetof(SphereCollider, centerOffset),
                vType,
                "glm, ::, vec3, (, 0.0f, )"
            };
            SphereCollider_Variables.push_back(std::move(rv));
        }
        ci.variables = SphereCollider_Variables;
        Reflection::Registry::Instance().RegisterComponent(std::move(ci));
    }
};
static SphereCollider_AutoRegister _spherecollider_autoreg;

} // namespace ReflectionGenerated

