// Auto-generated reflection file for SkeletalMeshComponent.h
#include "SkeletalMeshComponent.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {


static std::vector<Reflection::ReflectedVariable> SkeletalMeshComponent_Variables;
struct SkeletalMeshComponent_AutoRegister {
    SkeletalMeshComponent_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "SkeletalMeshComponent";
        ci.fullName = "SkeletalMeshComponent";
        ci.module = "/Script/GeneratedModule";
        ci.size = sizeof(SkeletalMeshComponent);
        ci.category = Reflection::TypeCategory::Struct;
        ci.isClass = false;
        ci.isStruct = true;
        ci.construct = []() -> void* { return new SkeletalMeshComponent(); };
        ci.destruct = [](void* p) { delete static_cast<SkeletalMeshComponent*>(p); };
        SkeletalMeshComponent_Variables.clear();
        auto* vType = Reflection::Registry::Instance().GetOrCreateType("std::basic_string<char>");
        {
            Reflection::ReflectedVariable rv = {
                "AssetPath", "public",
                false,
                offsetof(SkeletalMeshComponent, AssetPath),
                vType
            };
            SkeletalMeshComponent_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("int");
        {
            Reflection::ReflectedVariable rv = {
                "MaterialId", "public",
                false,
                offsetof(SkeletalMeshComponent, MaterialId),
                vType,
                "-, 1"
            };
            SkeletalMeshComponent_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "AnimationSpeed", "public",
                false,
                offsetof(SkeletalMeshComponent, AnimationSpeed),
                vType,
                "1.0f"
            };
            SkeletalMeshComponent_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("bool");
        {
            Reflection::ReflectedVariable rv = {
                "IsLooping", "public",
                false,
                offsetof(SkeletalMeshComponent, IsLooping),
                vType,
                "true"
            };
            SkeletalMeshComponent_Variables.push_back(std::move(rv));
        }
        ci.variables = SkeletalMeshComponent_Variables;
        Reflection::Registry::Instance().RegisterComponent(std::move(ci));
    }
};
static SkeletalMeshComponent_AutoRegister _skeletalmeshcomponent_autoreg;

} // namespace ReflectionGenerated

