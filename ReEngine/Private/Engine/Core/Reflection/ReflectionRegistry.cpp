#include "Engine/Core/Reflection/ReflectionRegistry.h"

static ReflectionRegistry g_registry;

ReflectionRegistry* ReflectionRegistry::GetGlobal() {
    return &g_registry; 
}

void ReflectionRegistry::registerClass(const ReflectionClass& classMeta) {
    classes[classMeta.name] = classMeta;
}

ReflectionClass* ReflectionRegistry::getClass(const std::string& className) {
    auto it = classes.find(className);
    return (it != classes.end()) ? &it->second : nullptr;
}