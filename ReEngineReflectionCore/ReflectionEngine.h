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
        const char* name;         // Short name
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

        FunctionPtr invoke; // wrapper that calls into the real method (or hook)
    };

    struct ReflectedVariable {
        const char* name;
        const char* access;
        bool is_static;
        std::size_t offset;

        const TypeInfo* type;
    };

    struct BaseClassInfo {
        const ClassInfo* baseType;
        std::size_t offset; // offset from derived to base
    };

    struct ClassInfo : public TypeInfo {
        const char* fullName;     // Fully qualified name (namespace/module/class)
        const char* module;

        std::function<void* (void)> construct;
        std::function<void(void*)> destruct;

        std::vector<BaseClassInfo> bases;      // Multiple inheritance support
        std::vector<ReflectedFunction> functions;
        std::vector<ReflectedVariable> variables;

        ClassInfo() { isClass = true; }
    };

    class REFLECT_API Registry {
    public:
        static Registry& Instance();

        // Register a class produced by generated code.
        void RegisterClass(ClassInfo&& info);
        void RegisterComponent(ClassInfo&& info);
        void RegisterSystem(ClassInfo&& info);

        // Query
        const ClassInfo* FindClass(const std::string& fullName) const; // expects Module.Name or just Name
        std::vector<const ClassInfo*> GetAllClasses() const;

        // Query
        const ClassInfo* FindComponent(const std::string& fullName) const; // expects Module.Name or just Name
        std::vector<const ClassInfo*> GetAllComponents() const;

        // Query
        const ClassInfo* FindSystem(const std::string& fullName) const; // expects Module.Name or just Name
        std::vector<const ClassInfo*> GetAllSystems() const;

        // Hook setter helper (generated code may export an implementation per-class
        // but we expose a convenience function centralised here too)
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
        std::unordered_map<std::string, ClassInfo> classes_; // keyed by Module.Name and by Name (if unique)
        std::unordered_map<std::string, ClassInfo> components_;
        std::unordered_map<std::string, ClassInfo> systems_;

        std::unordered_map<std::string, TypeInfo> types_;
    };

    // C-style exports that generated files can call if they can't link C++ symbols
    extern "C" {
        REFLECT_API void Reflection_RegisterClass(const ClassInfo* info);
        REFLECT_API void Reflection_SetHook(const char* className, const char* functionName, FunctionPtr hook);
    }

} // namespace Reflection