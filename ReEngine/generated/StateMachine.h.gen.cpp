// Auto-generated reflection file for StateMachine.h
#include "StateMachine.h"
#include "ReflectionEngine.h"
#include <cstddef>

namespace ReflectionGenerated {


static std::vector<Reflection::ReflectedVariable> StateMachine_Variables;
struct StateMachine_AutoRegister {
    StateMachine_AutoRegister() {
        Reflection::ClassInfo ci;
        ci.name = "StateMachine";
        ci.fullName = "StateMachine";
        ci.module = "/Script/GeneratedModule";
        ci.size = sizeof(StateMachine);
        ci.category = Reflection::TypeCategory::Struct;
        ci.isClass = false;
        ci.isStruct = true;
        ci.construct = []() -> void* { return new StateMachine(); };
        ci.destruct = [](void* p) { delete static_cast<StateMachine*>(p); };
        StateMachine_Variables.clear();
        auto* vType = Reflection::Registry::Instance().GetOrCreateType("std::basic_string<char>");
        {
            Reflection::ReflectedVariable rv = {
                "GraphAssetPath", "public",
                false,
                offsetof(StateMachine, GraphAssetPath),
                vType
            };
            StateMachine_Variables.push_back(std::move(rv));
        }
         vType = Reflection::Registry::Instance().GetOrCreateType("std::unordered_map<std::basic_string<char>, AnimVar>");
        {
            Reflection::ReflectedVariable rv = {
                "Blackboard", "public",
                false,
                offsetof(StateMachine, Blackboard),
                vType
            };
            StateMachine_Variables.push_back(std::move(rv));
        }
        ci.variables = StateMachine_Variables;
        Reflection::Registry::Instance().RegisterComponent(std::move(ci));
    }
};
static StateMachine_AutoRegister _statemachine_autoreg;

} // namespace ReflectionGenerated

