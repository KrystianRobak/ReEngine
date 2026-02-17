#include "ReflectionEngine.h"
#include "Logger.h"
#include <iostream>
#include <cstring>

using namespace Reflection;

Registry& Registry::Instance() {
    static Registry inst;
    return inst;
}

void Reflection::Registry::RegisterClassInstance(const std::string& className, void* instance)
{
    auto it = systems_.find(className);
    if (it != systems_.end()) {

    }
}

void Reflection::Registry::RegisterVariableInstance(const std::string& className, const std::string& variableName, void* instance)
{
    auto it = classes_.find(className);
    if (it != classes_.end()) {
        for (auto var : it->second.variables)
        {

        }
    }
}

void Reflection::Registry::UnregisterModule(const std::string& moduleName) {
    auto cleanup = [&](std::unordered_map<std::string, ClassInfo>& container) {
        for (auto it = container.begin(); it != container.end(); ) {
            if (it->second.module == moduleName) {
                LOGF_INFO("[ReflectionCore] Unregistered %s from %s", it->second.name, moduleName.c_str());
                it = container.erase(it);
            }
            else {
                ++it;
            }
        }
        };

    cleanup(classes_);
    cleanup(components_);
    cleanup(systems_);
}

void Reflection::Registry::ClearAll() {
    ClearSystems();
    ClearComponents();
    ClearClasses();
    types_.clear();
    LOGF_INFO("[ReflectionCore] Registry completely cleared.");
}

void Reflection::Registry::ClearAllExcept(const std::string& moduleToKeep) {
    auto cleanup = [&](std::unordered_map<std::string, ClassInfo>& container) {
        for (auto it = container.begin(); it != container.end(); ) {
            if (it->second.module != moduleToKeep) {
                LOGF_INFO("[ReflectionCore] Clearing %s (Module: %s)", it->second.name, it->second.module);
                it = container.erase(it);
            }
            else {
                ++it;
            }
        }
        };

    cleanup(classes_);
    cleanup(components_);
    cleanup(systems_);

    LOGF_INFO("[ReflectionCore] Registry cleared (kept module: %s).", moduleToKeep.c_str());
}

void Reflection::Registry::ClearSystems() {
    size_t count = systems_.size();
    systems_.clear();
    LOGF_INFO("[ReflectionCore] Cleared %zu Systems.", count);
}

void Reflection::Registry::ClearComponents() {
    size_t count = components_.size();
    components_.clear();
    LOGF_INFO("[ReflectionCore] Cleared %zu Components.", count);
}

void Reflection::Registry::ClearClasses() {
    size_t count = classes_.size();
    classes_.clear();
    LOGF_INFO("[ReflectionCore] Cleared %zu Classes.", count);
}

void Reflection::Registry::RegisterClass(ClassInfo&& info) {
    std::string key = sizeof(info.module) > 0 ? (std::string(info.module) + "." + info.name) : info.name;
    classes_.emplace(key, std::move(info));
    auto& entry = classes_.find(key)->second;
    if (classes_.find(entry.name) == classes_.end()) {
        classes_.emplace(entry.name, entry);
    }
    LOGF_INFO("[ReflectionCore] Registered Class %s", entry.name)
}

void Reflection::Registry::RegisterComponent(ClassInfo&& info) {
    std::string key = sizeof(info.module) > 0 ? (std::string(info.module) + "." + info.name) : info.name;
    components_.emplace(key, std::move(info));
    auto& entry = components_.find(key)->second;
    if (components_.find(entry.name) == components_.end()) {
        components_.emplace(entry.name, entry);
    }
    LOGF_INFO("[ReflectionCore] Registered Component %s", entry.name)
}

void Reflection::Registry::RegisterSystem(ClassInfo&& info) {
    std::string key = sizeof(info.module) > 0 ? (std::string(info.module) + "." + info.name) : info.name;
    systems_.emplace(key, std::move(info));
    auto& entry = systems_.find(key)->second;
    if (systems_.find(entry.name) == systems_.end()) {
        systems_.emplace(entry.name, entry);
    }
    LOGF_INFO("[ReflectionCore] Registered System %s", entry.name)
}

const ClassInfo* Reflection::Registry::FindClass(const std::string& fullName) const {
    auto it = classes_.find(fullName);
    if (it != classes_.end()) return &it->second;
    return nullptr;
}

std::vector<const ClassInfo*> Reflection::Registry::GetAllClasses() const {
    std::vector<const ClassInfo*> out;
    out.reserve(classes_.size());
    for (auto& p : classes_) {
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
        if (p.first.find('.') != std::string::npos) out.push_back(&p.second);
    }
    return out;
}

void Registry::SetHook(const char* className, const char* functionName, FunctionPtr hook) {
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
        (void)className; (void)functionName; (void)hook;
    }

}

