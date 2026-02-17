// Auto-generated reflection file for BoxCollider.h
#include "BoxCollider.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {


static std::vector<Reflection::ReflectedVariable> BoxCollider_Variables;
struct BoxCollider_AutoRegister {
    BoxCollider_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "BoxCollider";
        ci.fullName = "BoxCollider";
        ci.module = "ReEngine";
        ci.size = sizeof(BoxCollider);
        ci.category = Reflection::TypeCategory::Struct;
        ci.isClass = false;
        ci.isStruct = true;
        ci.construct = []() -> void* { return new BoxCollider(); };
        ci.destruct = [](void* p) { delete static_cast<BoxCollider*>(p); };
        BoxCollider_Variables.clear();
        auto* vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "offset", "public",
                false,
                offsetof(BoxCollider, offset),
                vType,
                "glm, ::, vec3, (, 0.0f, )"
            };
            BoxCollider_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "size", "public",
                false,
                offsetof(BoxCollider, size),
                vType,
                "glm, ::, vec3, (, 1.0f, )"
            };
            BoxCollider_Variables.push_back(std::move(rv));
        }
        ci.variables = BoxCollider_Variables;
        Reflection::Registry::Instance().RegisterComponent(std::move(ci));
    }
};
static BoxCollider_AutoRegister _boxcollider_autoreg;

} // namespace ReflectionGenerated

