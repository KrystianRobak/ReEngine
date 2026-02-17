// Auto-generated reflection file for LightSource.h
#include "LightSource.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {


static std::vector<Reflection::ReflectedVariable> LightSource_Variables;
struct LightSource_AutoRegister {
    LightSource_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "LightSource";
        ci.fullName = "LightSource";
        ci.module = "ReEngine";
        ci.size = sizeof(LightSource);
        ci.category = Reflection::TypeCategory::Struct;
        ci.isClass = false;
        ci.isStruct = true;
        ci.construct = []() -> void* { return new LightSource(); };
        ci.destruct = [](void* p) { delete static_cast<LightSource*>(p); };
        LightSource_Variables.clear();
        auto* vType = Reflection::Registry::Instance().GetOrCreateType("int");
        {
            Reflection::ReflectedVariable rv = {
                "type", "public",
                false,
                offsetof(LightSource, type),
                vType,
                "static_cast, <, int, >, (, LightType, ::, Directional, )"
            };
            LightSource_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "LightColor", "public",
                false,
                offsetof(LightSource, LightColor),
                vType,
                "glm, ::, vec3, (, 1.0f, 1.0f, 1.0f, )"
            };
            LightSource_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "intensity", "public",
                false,
                offsetof(LightSource, intensity),
                vType,
                "1.0f"
            };
            LightSource_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "Ambient", "public",
                false,
                offsetof(LightSource, Ambient),
                vType,
                "glm, ::, vec3, (, 0.1f, )"
            };
            LightSource_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "Diffuse", "public",
                false,
                offsetof(LightSource, Diffuse),
                vType,
                "glm, ::, vec3, (, 1.0f, )"
            };
            LightSource_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "Specular", "public",
                false,
                offsetof(LightSource, Specular),
                vType,
                "glm, ::, vec3, (, 1.0f, )"
            };
            LightSource_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "constant", "public",
                false,
                offsetof(LightSource, constant),
                vType,
                "1.0f"
            };
            LightSource_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "linear", "public",
                false,
                offsetof(LightSource, linear),
                vType,
                "0.09f"
            };
            LightSource_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "quadratic", "public",
                false,
                offsetof(LightSource, quadratic),
                vType,
                "0.032f"
            };
            LightSource_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "cutOff", "public",
                false,
                offsetof(LightSource, cutOff),
                vType,
                "glm, ::, cos, (, glm, ::, radians, (, 12.5f, ), )"
            };
            LightSource_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "outerCutOff", "public",
                false,
                offsetof(LightSource, outerCutOff),
                vType,
                "glm, ::, cos, (, glm, ::, radians, (, 15.0f, ), )"
            };
            LightSource_Variables.push_back(std::move(rv));
        }
        ci.variables = LightSource_Variables;
        Reflection::Registry::Instance().RegisterComponent(std::move(ci));
    }
};
static LightSource_AutoRegister _lightsource_autoreg;

} // namespace ReflectionGenerated

