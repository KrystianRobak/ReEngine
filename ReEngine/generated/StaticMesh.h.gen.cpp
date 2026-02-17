// Auto-generated reflection file for StaticMesh.h
#include "StaticMesh.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {


static std::vector<Reflection::ReflectedVariable> StaticMesh_Variables;
struct StaticMesh_AutoRegister {
    StaticMesh_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "StaticMesh";
        ci.fullName = "StaticMesh";
        ci.module = "ReEngine";
        ci.size = sizeof(StaticMesh);
        ci.category = Reflection::TypeCategory::Struct;
        ci.isClass = false;
        ci.isStruct = true;
        ci.construct = []() -> void* { return new StaticMesh(); };
        ci.destruct = [](void* p) { delete static_cast<StaticMesh*>(p); };
        StaticMesh_Variables.clear();
        auto* vType = Reflection::Registry::Instance().GetOrCreateType("std::basic_string<char>");
        {
            Reflection::ReflectedVariable rv = {
                "AssetPath", "public",
                false,
                offsetof(StaticMesh, AssetPath),
                vType
            };
            StaticMesh_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("int");
        {
            Reflection::ReflectedVariable rv = {
                "MaterialId", "public",
                false,
                offsetof(StaticMesh, MaterialId),
                vType,
                "-, 1"
            };
            StaticMesh_Variables.push_back(std::move(rv));
        }
        ci.variables = StaticMesh_Variables;
        Reflection::Registry::Instance().RegisterComponent(std::move(ci));
    }
};
static StaticMesh_AutoRegister _staticmesh_autoreg;

} // namespace ReflectionGenerated

