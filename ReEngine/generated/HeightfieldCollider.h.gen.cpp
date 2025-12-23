// Auto-generated reflection file for HeightfieldCollider.h
#include "HeightfieldCollider.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {


static std::vector<Reflection::ReflectedVariable> HeightfieldCollider_Variables;
struct HeightfieldCollider_AutoRegister {
    HeightfieldCollider_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "HeightfieldCollider";
        ci.fullName = "HeightfieldCollider";
        ci.module = "/Script/GeneratedModule";
        ci.size = sizeof(HeightfieldCollider);
        ci.category = Reflection::TypeCategory::Struct;
        ci.isClass = false;
        ci.isStruct = true;
        ci.construct = []() -> void* { return new HeightfieldCollider(); };
        ci.destruct = [](void* p) { delete static_cast<HeightfieldCollider*>(p); };
        HeightfieldCollider_Variables.clear();
        auto* vType = Reflection::Registry::Instance().GetOrCreateType("int");
        {
            Reflection::ReflectedVariable rv = {
                "width", "public",
                false,
                offsetof(HeightfieldCollider, width),
                vType,
                "0"
            };
            HeightfieldCollider_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("int");
        {
            Reflection::ReflectedVariable rv = {
                "depth", "public",
                false,
                offsetof(HeightfieldCollider, depth),
                vType,
                "0"
            };
            HeightfieldCollider_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "scale", "public",
                false,
                offsetof(HeightfieldCollider, scale),
                vType,
                "0.0f"
            };
            HeightfieldCollider_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "heightMultiplier", "public",
                false,
                offsetof(HeightfieldCollider, heightMultiplier),
                vType,
                "0.0f"
            };
            HeightfieldCollider_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "halfWidth", "public",
                false,
                offsetof(HeightfieldCollider, halfWidth),
                vType,
                "0.0f"
            };
            HeightfieldCollider_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "halfDepth", "public",
                false,
                offsetof(HeightfieldCollider, halfDepth),
                vType,
                "0.0f"
            };
            HeightfieldCollider_Variables.push_back(std::move(rv));
        }
        ci.variables = HeightfieldCollider_Variables;
        Reflection::Registry::Instance().RegisterComponent(std::move(ci));
    }
};
static HeightfieldCollider_AutoRegister _heightfieldcollider_autoreg;

} // namespace ReflectionGenerated

