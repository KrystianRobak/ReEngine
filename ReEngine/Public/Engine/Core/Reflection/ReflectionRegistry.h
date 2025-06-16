#pragma once
#include <string>
#include <unordered_map>
#include <vector>

using FunctionPtr = void(*)(void*, void**, void*);
struct ReflectionMethod {
    std::string_view name;
    std::string_view return_type;
    std::string_view access;
    bool is_static;
    bool is_virtual;
    bool is_const;
    FunctionPtr invoke;
};

struct ReflectionVariable {
    std::string_view name;
    std::string_view type;
    std::string_view access;
    bool is_const;
    bool is_static;
    bool is_pointer;
    bool is_reference;
    size_t offset;
};

struct ReflectionClass {
    std::string name;
    std::vector<ReflectionMethod> methods;
    std::vector<ReflectionVariable> properties;
};

class ReflectionRegistry {
public:
    std::unordered_map<std::string, ReflectionClass> classes;

    void registerClass(const ReflectionClass& classMetadata) {
        classes[classMetadata.name] = classMetadata;
    }

    ReflectionClass* getClass(const std::string& className) {
        auto it = classes.find(className);
        return (it != classes.end()) ? &it->second : nullptr;
    }

    static ReflectionRegistry* GetGlobal();
};

