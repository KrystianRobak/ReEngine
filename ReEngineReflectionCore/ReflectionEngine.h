#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

#if defined(REFLECT_CORE_EXPORTS)
#define REFLECT_API __declspec(dllexport)
#else
#define REFLECT_API __declspec(dllimport)
#endif


template <typename Derived, typename Base>
static std::size_t __rg_BaseOffset() {
    return reinterpret_cast<std::size_t>(static_cast<Base*>(reinterpret_cast<Derived*>(1))) - 1;
}

namespace Reflection {

    class ClassInfo;

    using FunctionPtr = void(*)(void*, void**, void*);

    enum class TypeCategory {
        Primitive,
        Class,
        Struct,
        Enum,
        Pointer,
        Reference,
        Array,
        Function,
        Unknown
    };

    struct TypeInfo {
        const char* name;
        std::size_t size = 0;
        TypeCategory category = TypeCategory::Unknown;
        bool isClass = false;
        bool isStruct = false;
        bool isEnum = false;
        bool isPointer = false;
        bool isReference = false;

        virtual ~TypeInfo() = default;
    };

    struct ReflectedFunction {
        const char* name;
        const char* access;
        bool is_static;
        bool is_virtual;
        bool is_const;

        const TypeInfo* returnType;
        std::vector<const TypeInfo*> paramTypes;

        FunctionPtr invoke;
    };

    struct ReflectedVariable {
        const char* name;
        const char* access;
        bool is_static;
        std::size_t offset;

        const TypeInfo* type;
        std::string defaultValue;
    };

    struct BaseClassInfo {
        const ClassInfo* baseType;
        std::size_t offset;
    };

    struct ClassInfo : public TypeInfo {
        const char* fullName;
        const char* module;

        std::function<void* (void)> construct;
        std::function<void(void*)> destruct;

        std::vector<BaseClassInfo> bases;
        std::vector<ReflectedFunction> functions;
        std::vector<ReflectedVariable> variables;

        ClassInfo() { isClass = true; }
    };

    class REFLECT_API Registry {
    public:
        static Registry& Instance();

        void RegisterClassInstance(const std::string& className, void* instance);

        void RegisterVariableInstance(const std::string& className, const std::string& variableName, void* instance);

        void UnregisterModule(const std::string& moduleName);

        void ClearAll();
        void ClearAllExcept(const std::string& moduleToKeep);

        void ClearSystems();
        void ClearComponents();
        void ClearClasses();

        void RegisterClass(ClassInfo&& info);
        void RegisterComponent(ClassInfo&& info);
        void RegisterSystem(ClassInfo&& info);

        const ClassInfo* FindClass(const std::string& fullName) const;
        std::vector<const ClassInfo*> GetAllClasses() const;

        const ClassInfo* FindComponent(const std::string& fullName) const;
        std::vector<const ClassInfo*> GetAllComponents() const;

        const ClassInfo* FindSystem(const std::string& fullName) const; 
        std::vector<const ClassInfo*> GetAllSystems() const;

        void SetHook(const char* className, const char* functionName, FunctionPtr hook);
         const TypeInfo* GetOrCreateType(const char* name,
                                    std::size_t size = 0,
                                    TypeCategory category = TypeCategory::Unknown,
                                    bool isClass = false,
                                    bool isStruct = false,
                                    bool isEnum = false,
                                    bool isPointer = false,
                                    bool isReference = false);
    private:
        std::unordered_map<std::string, ClassInfo> classes_;
        std::unordered_map<std::string, ClassInfo> components_;
        std::unordered_map<std::string, ClassInfo> systems_;

        std::unordered_map<std::string, TypeInfo> types_;
    };

    extern "C" {
        REFLECT_API void Reflection_RegisterClass(const ClassInfo* info);
        REFLECT_API void Reflection_SetHook(const char* className, const char* functionName, FunctionPtr hook);
    }

}