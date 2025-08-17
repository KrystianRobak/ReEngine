#pragma once
#include "ReflectionEngine.h"
#include <iostream>


void InspectClass(const Reflection::ClassInfo* c);

struct VariableHelper {
    const Reflection::ReflectedVariable* var;
    void* instance;

    VariableHelper(const Reflection::ReflectedVariable* v, void* inst)
        : var(v), instance(inst) {}

    void Print() const {
        std::cout << var->name
            << " (" << (var->type ? var->type->name : "<null>") << ") "
            << "offset=" << var->offset << " -> ";

        if (!var->type) {
            std::cout << "<unknown type>";
        }
        else if (strcmp(var->type->name, "int") == 0) {
            int val = *reinterpret_cast<int*>(
                reinterpret_cast<char*>(instance) + var->offset);
            std::cout << val;
        }
        else if (strcmp(var->type->name, "float") == 0) {
            float val = *reinterpret_cast<float*>(
                reinterpret_cast<char*>(instance) + var->offset);
            std::cout << val;
        }
        else if (strcmp(var->type->name, "std::string") == 0) {
            std::string& s = *reinterpret_cast<std::string*>(
                reinterpret_cast<char*>(instance) + var->offset);
            std::cout << s;
        }
        else {
            std::cout << "<unknown type>";
        }
        std::cout << "\n";
    }
};

struct FunctionHelper {
    const Reflection::ReflectedFunction* fun;
    void* instance;

    FunctionHelper(const Reflection::ReflectedFunction* f, void* inst)
        : fun(f), instance(inst) {}

    void PrintSignature() const {
        std::cout << fun->name << " -> returns "
            << (fun->returnType ? fun->returnType->name : "void") << " (";
        for (size_t pi = 0; pi < fun->paramTypes.size(); ++pi) {
            if (pi > 0) std::cout << ", ";
            std::cout << fun->paramTypes[pi]->name;
        }
        std::cout << ")\n";
    }

    template<typename Ret, typename... Args>
    Ret Invoke(Args... args) const {
        Ret ret{};
        void* arguments[] = { &args... };
        if (fun->invoke) {
            fun->invoke(instance, arguments, &ret);
        }
        return ret;
    }
};

void PrintVariables(void* instance, const std::vector<Reflection::ReflectedVariable>& vars) {
    for (size_t i = 0; i < vars.size(); ++i) {
        VariableHelper(&vars[i], instance).Print();
    }
}

void PrintFunctions(void* instance, const std::vector<Reflection::ReflectedFunction>& funcs) {
    for (size_t i = 0; i < funcs.size(); ++i) {
        FunctionHelper(&funcs[i], instance).PrintSignature();
    }
}

void PrintInheritance(const Reflection::ClassInfo* c, int indent = 0) {
    for (int i = 0; i < indent; ++i) std::cout << "  ";

    std::cout << "- " << c->module << "." << c->name << "\n";
    InspectClass(c);
    for (const auto& base : c->bases) {
        if (base.baseType)
        {
            PrintInheritance(base.baseType, indent + 1);

        }
    }
}

void InspectClass(const Reflection::ClassInfo* c) {
    std::cout << "Class: " << c->module << "." << c->name
        << " (size=" << c->size << ")\n";

    if (!c->bases.empty()) {
        std::cout << " Inherits:\n";
        for (const auto& base : c->bases) {
            std::cout << "  Base offset: " << base.offset << " ";
            PrintInheritance(base.baseType, 1);
        }
    }

    if (c->construct) {
        void* inst = c->construct();
        std::cout << "  Instance created at " << inst << "\n";

        if (!c->variables.empty()) {
            std::cout << " Variables:\n";
            PrintVariables(inst, c->variables);
        }

        if (!c->functions.empty()) {
            std::cout << " Functions:\n";
            PrintFunctions(inst, c->functions);
        }

        if (c->destruct) {
            c->destruct(inst);
        }
        else {
            delete reinterpret_cast<char*>(inst);
        }
    }

    std::cout << "\n";
}