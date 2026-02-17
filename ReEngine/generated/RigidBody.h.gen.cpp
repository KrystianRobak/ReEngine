// Auto-generated reflection file for RigidBody.h
#include "RigidBody.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {


static std::vector<Reflection::ReflectedVariable> RigidBody_Variables;
struct RigidBody_AutoRegister {
    RigidBody_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "RigidBody";
        ci.fullName = "RigidBody";
        ci.module = "ReEngine";
        ci.size = sizeof(RigidBody);
        ci.category = Reflection::TypeCategory::Struct;
        ci.isClass = false;
        ci.isStruct = true;
        ci.construct = []() -> void* { return new RigidBody(); };
        ci.destruct = [](void* p) { delete static_cast<RigidBody*>(p); };
        RigidBody_Variables.clear();
        auto* vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "velocity", "public",
                false,
                offsetof(RigidBody, velocity),
                vType,
                "glm, ::, vec3, (, 0.0f, )"
            };
            RigidBody_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "acceleration", "public",
                false,
                offsetof(RigidBody, acceleration),
                vType,
                "glm, ::, vec3, (, 0.0f, )"
            };
            RigidBody_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "mass", "public",
                false,
                offsetof(RigidBody, mass),
                vType,
                "1.0f"
            };
            RigidBody_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "friction", "public",
                false,
                offsetof(RigidBody, friction),
                vType,
                "0.5f"
            };
            RigidBody_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "restitution", "public",
                false,
                offsetof(RigidBody, restitution),
                vType,
                "0.0f"
            };
            RigidBody_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("bool");
        {
            Reflection::ReflectedVariable rv = {
                "useGravity", "public",
                false,
                offsetof(RigidBody, useGravity),
                vType,
                "true"
            };
            RigidBody_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("bool");
        {
            Reflection::ReflectedVariable rv = {
                "isStatic", "public",
                false,
                offsetof(RigidBody, isStatic),
                vType,
                "false"
            };
            RigidBody_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "angularVelocity", "public",
                false,
                offsetof(RigidBody, angularVelocity),
                vType,
                "glm, ::, vec3, (, 0.0f, )"
            };
            RigidBody_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("bool");
        {
            Reflection::ReflectedVariable rv = {
                "lockAngular", "public",
                false,
                offsetof(RigidBody, lockAngular),
                vType,
                "false"
            };
            RigidBody_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("float");
        {
            Reflection::ReflectedVariable rv = {
                "inverseMass", "public",
                false,
                offsetof(RigidBody, inverseMass),
                vType
            };
            RigidBody_Variables.push_back(std::move(rv));
        }
        ci.variables = RigidBody_Variables;
        Reflection::Registry::Instance().RegisterComponent(std::move(ci));
    }
};
static RigidBody_AutoRegister _rigidbody_autoreg;

} // namespace ReflectionGenerated

