#pragma once
#include <string>
#include <unordered_map>
#include <vector>


struct ReflectionVariable
{
    std::string name;
    std::string type;
};

struct ReflectionMethod {
    std::string name;
    std::string returnType;
    std::vector<ReflectionVariable> parameters;
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
};

