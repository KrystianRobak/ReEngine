// Auto-generated reflection file for ReFold/Rendering/Sprite.h
#include "ReFold/Rendering/Sprite.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {


static std::vector<Reflection::ReflectedVariable> Sprite_Variables;
struct Sprite_AutoRegister {
    Sprite_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "Sprite";
        ci.fullName = "Sprite";
        ci.module = "/Script/GeneratedModule";
        ci.size = sizeof(Sprite);
        ci.category = Reflection::TypeCategory::Struct;
        ci.isClass = false;
        ci.isStruct = true;
        ci.construct = []() -> void* { return new Sprite(); };
        ci.destruct = [](void* p) { delete static_cast<Sprite*>(p); };
        Sprite_Variables.clear();
        auto* vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "position", "public",
                false,
                offsetof(Sprite, position),
                vType
            };
            Sprite_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::qua<float>");
        {
            Reflection::ReflectedVariable rv = {
                "rotation", "public",
                false,
                offsetof(Sprite, rotation),
                vType
            };
            Sprite_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("glm::vec<3, float>");
        {
            Reflection::ReflectedVariable rv = {
                "scale", "public",
                false,
                offsetof(Sprite, scale),
                vType
            };
            Sprite_Variables.push_back(std::move(rv));
        }
        ci.variables = Sprite_Variables;
        Reflection::Registry::Instance().RegisterComponent(std::move(ci));
    }
};
static Sprite_AutoRegister _sprite_autoreg;

} // namespace ReflectionGenerated

