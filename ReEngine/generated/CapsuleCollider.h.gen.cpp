// Auto-generated reflection file for CapsuleCollider.h
#include "CapsuleCollider.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {


static std::vector<Reflection::ReflectedVariable> CapsuleCollider_Variables;
struct CapsuleCollider_AutoRegister {
    CapsuleCollider_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "CapsuleCollider";
        ci.fullName = "CapsuleCollider";
        ci.module = "ReEngine";
        ci.size = sizeof(CapsuleCollider);
        ci.category = Reflection::TypeCategory::Struct;
        ci.isClass = false;
        ci.isStruct = true;
        ci.construct = []() -> void* { return new CapsuleCollider(); };
        ci.destruct = [](void* p) { delete static_cast<CapsuleCollider*>(p); };
        CapsuleCollider_Variables.clear();
        auto* vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "radius", "public",
                false,
                offsetof(CapsuleCollider, radius),
                vType,
                "0.5f"
            };
            CapsuleCollider_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "height", "public",
                false,
                offsetof(CapsuleCollider, height),
                vType,
                "2.0f"
            };
            CapsuleCollider_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "offset", "public",
                false,
                offsetof(CapsuleCollider, offset),
                vType,
                "glm, ::, vec3, (, 0.0f, )"
            };
            CapsuleCollider_Variables.push_back(std::move(rv));
        }
        ci.variables = CapsuleCollider_Variables;
        Reflection::Registry::Instance().RegisterComponent(std::move(ci));
    }
};
static CapsuleCollider_AutoRegister _capsulecollider_autoreg;

} // namespace ReflectionGenerated

