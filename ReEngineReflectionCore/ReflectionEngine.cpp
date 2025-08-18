#include "ReflectionEngine.h"

#include <iostream>
#include <cstring>

using namespace Reflection;

Registry& Registry::Instance() {
    static Registry inst;
    return inst;
}

void Registry::RegisterClass(ClassInfo&& info) {
    std::string key = sizeof(info.module) > 0 ? (std::string(info.module) + "." + info.name) : info.name;
    classes_.emplace(key, std::move(info));
    // Also store by plain name if name not present to simplify lookups in small projects.
    auto& entry = classes_.find(key)->second;
    if (classes_.find(entry.name) == classes_.end()) {
        classes_.emplace(entry.name, entry);
    }
    std::cout << "[ReflectionCore] Registered: " << key << "";
}

void Registry::RegisterComponent(ClassInfo&& info) {
    std::string key = sizeof(info.module) > 0 ? (std::string(info.module) + "." + info.name) : info.name;
    components_.emplace(key, std::move(info));
    // Also store by plain name if not present to simplify lookups
    auto& entry = components_.find(key)->second;
    if (components_.find(entry.name) == components_.end()) {
        components_.emplace(entry.name, entry);
    }
    std::cout << "[ReflectionCore] Registered Component: " << key << "\n";
}

void Registry::RegisterSystem(ClassInfo&& info) {
    std::string key = sizeof(info.module) > 0 ? (std::string(info.module) + "." + info.name) : info.name;
    systems_.emplace(key, std::move(info));
    // Also store by plain name if not present to simplify lookups
    auto& entry = systems_.find(key)->second;
    if (systems_.find(entry.name) == systems_.end()) {
        systems_.emplace(entry.name, entry);
    }
    std::cout << "[ReflectionCore]" << " Registered System: " << key << "\n";
}

const ClassInfo* Registry::FindClass(const std::string& fullName) const {
    auto it = classes_.find(fullName);
    if (it != classes_.end()) return &it->second;
    return nullptr;
}

std::vector<const ClassInfo*> Registry::GetAllClasses() const {
    std::vector<const ClassInfo*> out;
    out.reserve(classes_.size());
    for (auto& p : classes_) {
        // store only entries that include module (to avoid duplicates)
        if (p.first.find('.') != std::string::npos) out.push_back(&p.second);
    }
    return out;
}

const ClassInfo* Reflection::Registry::FindComponent(const std::string& fullName) const
{
    auto it = components_.find(fullName);
    if (it != components_.end()) return &it->second;
    return nullptr;
}

std::vector<const ClassInfo*> Reflection::Registry::GetAllComponents() const
{
    std::vector<const ClassInfo*> out;
    out.reserve(components_.size());
    for (auto& p : components_) {
        // store only entries that include module (to avoid duplicates)
        if (p.first.find('.') != std::string::npos) out.push_back(&p.second);
    }
    return out;
}

const ClassInfo* Reflection::Registry::FindSystem(const std::string& fullName) const
{
    auto it = systems_.find(fullName);
    if (it != systems_.end()) return &it->second;
    return nullptr;
}

std::vector<const ClassInfo*> Reflection::Registry::GetAllSystems() const
{
    std::vector<const ClassInfo*> out;
    out.reserve(systems_.size());
    for (auto& p : systems_) {
        // store only entries that include module (to avoid duplicates)
        if (p.first.find('.') != std::string::npos) out.push_back(&p.second);
    }
    return out;
}

void Registry::SetHook(const char* className, const char* functionName, FunctionPtr hook) {
    // Default behavior: try to find a runtime exported hook setter. Otherwise, do nothing.
    // Generated code in modules typically implement Reflection::SetHook or C export to connect hooks.
    // For convenience attempt C-export as fallback.
    Reflection_SetHook(className, functionName, hook);
}

const TypeInfo* Reflection::Registry::GetOrCreateType(const char* name, std::size_t size, TypeCategory category, bool isClass, bool isStruct, bool isEnum, bool isPointer, bool isReference)
{
    auto it = types_.find(name);
    if (it != types_.end()) {
        return &it->second;
    }

    TypeInfo ti;
    ti.name = name;
    ti.size = size;
    ti.category = category;
    ti.isClass = isClass;
    ti.isStruct = isStruct;
    ti.isEnum = isEnum;
    ti.isPointer = isPointer;
    ti.isReference = isReference;

    auto [iter, _] = types_.emplace(name, std::move(ti));
    return &iter->second;
}

// C exports simply forward to the C++ registry. Generated code can call this easily.
extern "C" {

    REFLECT_API void Reflection_RegisterClass(const Reflection::ClassInfo* info) {
        if (!info) return;
        Reflection::ClassInfo local;
        local.name = info->name;
        local.module = info->module;
        local.size = info->size;
        local.construct = info->construct;
        local.functions = info->functions;
        local.variables = info->variables;
        Reflection::Registry::Instance().RegisterClass(std::move(local));
    }

    REFLECT_API void Reflection_SetHook(const char* className, const char* functionName, Reflection::FunctionPtr hook) {
        // No-op default: modules usually provide their own SetHook impl (linked into the module)
        (void)className; (void)functionName; (void)hook;
    }

} // extern "C"

