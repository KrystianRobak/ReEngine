import os
import sys
import argparse
from pathlib import Path
from collections import defaultdict
import clang.cindex


# Configure libclang path if necessary
clang.cindex.Config.set_library_file(r"D:\LLVM\bin\libclang.dll")


def normalize_type_spelling(spelling: str) -> str:
    return spelling.strip().replace("class ", "").replace("struct ", "").replace("enum ", "")


# ... (type_category_for and base_offset_expr functions remain the same) ...
def type_category_for(clang_type) -> str:
    from clang.cindex import TypeKind
    tk = clang_type.kind
    if tk in (
            TypeKind.VOID, TypeKind.BOOL,
            TypeKind.CHAR_U, TypeKind.UCHAR, TypeKind.CHAR_S, TypeKind.SCHAR,
            TypeKind.USHORT, TypeKind.SHORT, TypeKind.UINT, TypeKind.INT,
            TypeKind.ULONG, TypeKind.LONG, TypeKind.ULONGLONG, TypeKind.LONGLONG,
            TypeKind.FLOAT, TypeKind.DOUBLE, TypeKind.LONGDOUBLE,
            TypeKind.WCHAR, TypeKind.CHAR16, TypeKind.CHAR32
    ):
        return "Primitive"
    if tk == TypeKind.ENUM:
        return "Enum"
    if tk == TypeKind.RECORD:
        # This will be refined in the parser to distinguish class/struct
        return "Class"
    if tk == TypeKind.POINTER:
        return "Pointer"
    if tk in (TypeKind.LVALUEREFERENCE, TypeKind.RVALUEREFERENCE):
        return "Reference"
    if tk == TypeKind.CONSTANTARRAY:
        return "Array"
    if tk == TypeKind.FUNCTIONPROTO:
        return "Function"
    return "Unknown"


def base_offset_expr(derived: str, base: str) -> str:
    return f"(reinterpret_cast<std::size_t>(static_cast<{base}*>(reinterpret_cast<{derived}*>(1))) - 1)"


class ReflectionGenerator:
    def __init__(self):
        self.classes = []
        self.processed_files = []

    # ---------- Parsing ----------
    def parse(self, filepath: str, clang_parse_args=None):
        index = clang.cindex.Index.create()
        # <--- MODIFIED: Add all reflection macros here to be ignored by the parser
        final_args = clang_parse_args + [
            "-DREFLECT()=",
            "-DREFCOMPONENT()=",
            "-DREFSYSTEM()=",
            "-DREFFUNCTION()=",
            "-DREFVARIABLE()=",
        ]
        tu = index.parse(filepath, args=final_args)

        found_classes = False
        for cursor in tu.cursor.get_children():
            if not cursor.location.file or cursor.location.file.name != filepath:
                continue

            if cursor.kind in (clang.cindex.CursorKind.CLASS_DECL, clang.cindex.CursorKind.STRUCT_DECL):
                # <--- MODIFIED: Check for any of the valid reflection macros
                reflection_type = self._get_reflection_type(cursor)
                if cursor.spelling and reflection_type:
                    found_classes = True
                    cls = {
                        "name": cursor.spelling,
                        "full_name": cursor.type.spelling or cursor.spelling,
                        "reflection_type": reflection_type,  # <--- NEW: Store which macro was found
                        "is_struct": cursor.kind == clang.cindex.CursorKind.STRUCT_DECL,
                        "functions": [],
                        "variables": [],
                        "bases": [],
                    }

                    # ... (rest of the parsing for bases and members is unchanged) ...
                    # Bases
                    for c in cursor.get_children():
                        if c.kind == clang.cindex.CursorKind.CXX_BASE_SPECIFIER:
                            base_type = c.type.spelling
                            if base_type:
                                cls["bases"].append(base_type)

                    # Members
                    for c in cursor.get_children():
                        access = self._get_access_specifier(c)
                        if c.kind == clang.cindex.CursorKind.CXX_METHOD and self._has_ref_macro(c, "REFFUNCTION"):
                            params = [(normalize_type_spelling(arg.type.spelling), arg.spelling or f"param_{i}")
                                      for i, arg in enumerate(c.get_arguments())]
                            ret_spell = normalize_type_spelling(c.result_type.spelling)
                            cls["functions"].append({
                                "name": c.spelling,
                                "return_type": ret_spell,
                                "params": params,
                                "access": access,
                                "is_static": c.is_static_method(),
                                "is_virtual": c.is_virtual_method(),
                                "is_const": c.is_const_method()
                            })
                        elif c.kind == clang.cindex.CursorKind.FIELD_DECL and self._has_ref_macro(c, "REFVARIABLE"):
                            vspell = normalize_type_spelling(c.type.spelling)
                            cls["variables"].append({
                                "name": c.spelling,
                                "type": vspell,
                                "access": access,
                                "is_static": False,
                            })

                    if cls["functions"] or cls["variables"] or reflection_type:
                        self.classes.append((filepath, cls))

        if found_classes:
            self.processed_files.append(filepath)

    # <--- NEW: Replaced _has_reflect_macro with a more powerful version
    def _get_reflection_type(self, cursor) -> str | None:
        """Checks the line(s) before a cursor for a reflection macro and returns its type."""
        try:
            with open(cursor.location.file.name, "r", encoding="utf-8") as f:
                lines = f.readlines()
                # Check the line right before the class/struct definition
                if cursor.location.line > 1:
                    prev_line = lines[cursor.location.line - 2]
                    if "REFCOMPONENT" in prev_line:
                        return "Component"
                    if "REFSYSTEM" in prev_line:
                        return "System"
                    # REFLECT should be checked last as it's the most generic
                    if "REFLECT" in prev_line:
                        return "Class"
            return None
        except Exception:
            return None

    def _has_ref_macro(self, cursor, macro: str) -> bool:
        # ... (this function remains the same) ...
        try:
            with open(cursor.location.file.name, "r", encoding="utf-8") as f:
                lines = f.readlines()
                line_idx = cursor.location.line - 1
                for offset in (0, -1):
                    i = line_idx + offset
                    if 0 <= i < len(lines):
                        if macro in lines[i]:
                            return True
                return False
        except Exception:
            return False

    def _get_access_specifier(self, cursor) -> str:
        # ... (this function remains the same) ...
        a = cursor.access_specifier
        from clang.cindex import AccessSpecifier
        if a == AccessSpecifier.PUBLIC: return "public"
        if a == AccessSpecifier.PROTECTED: return "protected"
        if a == AccessSpecifier.PRIVATE: return "private"
        if cursor.semantic_parent and cursor.semantic_parent.kind == clang.cindex.CursorKind.STRUCT_DECL:
            return "public"
        return "private"

    # ---------- Emission ----------
    def generate_cpp_for_classes(self, out_path: str, header_file: str, classes):
        # ... (this function's start remains the same) ...
        with open(out_path, "w", encoding="utf-8") as f:
            f.write(f"// Auto-generated reflection file for {header_file}\n")
            f.write(f'#include "{header_file}"\n')
            f.write('#include "ReflectionEngine.h"\n')
            f.write("#include <cstddef>\n\n")

            f.write("namespace ReflectionGenerated {\n\n")

            for cls in classes:
                self._emit_class_block(f, cls)

            f.write("} // namespace ReflectionGenerated\n\n")

    def _emit_class_block(self, f, cls):
        class_name = cls["name"]
        # ... (most of this function remains the same until the registration call) ...
        funcCounter = 0
        # Hook pointers
        for func in cls["functions"]:
            f.write(f"static Reflection::FunctionPtr {class_name}_{func['name']}_Hook = nullptr;\n")
        f.write("\n")

        # Invokers
        for func in cls["functions"]:
            self._emit_function_invoker(f, class_name, func)

        # Functions
        if cls["functions"]:
            f.write(f"static std::vector<Reflection::ReflectedFunction> {class_name}_Functions;\n")

        # Variables
        if cls["variables"]:
            f.write(f"static std::vector<Reflection::ReflectedVariable> {class_name}_Variables;\n")

        # Bases
        if cls["bases"]:
            f.write(f"static std::vector<Reflection::BaseClassInfo> {class_name}_Bases;\n")

        # Auto-register
        f.write(f"struct {class_name}_AutoRegister {{\n")
        f.write(f"    {class_name}_AutoRegister() {{\n")
        f.write(f"        Reflection::ClassInfo ci;\n")
        f.write(f'        ci.name = "{class_name}";\n')
        f.write(f'        ci.fullName = "{cls["full_name"]}";\n')
        f.write(f'        ci.module = "/Script/GeneratedModule";\n')
        f.write(f"        ci.size = sizeof({class_name});\n")

        if cls["is_struct"]:
            f.write(f"        ci.category = Reflection::TypeCategory::Struct;\n")
            f.write(f"        ci.isClass = false;\n")
            f.write(f"        ci.isStruct = true;\n")
        else:
            f.write(f"        ci.category = Reflection::TypeCategory::Class;\n")
            f.write(f"        ci.isClass = true;\n")
            f.write(f"        ci.isStruct = false;\n")
        f.write(f"        ci.construct = []() -> void* {{ return new {class_name}(); }};\n")
        f.write(f"        ci.destruct = [](void* p) {{ delete static_cast<{class_name}*>(p); }};\n")

        # Bases
        if cls["bases"]:
            f.write(f"        {class_name}_Bases.clear();\n")
            for b in cls["bases"]:
                offset = base_offset_expr(class_name, b)
                f.write(f'        if (auto* __base = Reflection::Registry::Instance().FindClass("{b}"))\n')
                f.write(f'            {class_name}_Bases.push_back(Reflection::BaseClassInfo{{ __base, {offset} }});\n')
            f.write(f"        ci.bases = {class_name}_Bases;\n")

        # Functions
        if cls["functions"]:
            f.write(f"        {class_name}_Functions.clear();\n")
            for func in cls["functions"]:
                f.write("        {\n")
                f.write(
                    f'            auto* retType = Reflection::Registry::Instance().GetOrCreateType("{func["return_type"]}");\n')
                f.write("            std::vector<const Reflection::TypeInfo*> paramTypes;\n")
                for (t, _) in func["params"]:
                    f.write(
                        f'            paramTypes.push_back(Reflection::Registry::Instance().GetOrCreateType("{t}"));\n')
                f.write("            Reflection::ReflectedFunction rf = {\n")
                f.write(f'                "{func["name"]}", "{func["access"]}",\n')
                f.write(
                    f"                {str(func['is_static']).lower()}, {str(func['is_virtual']).lower()}, {str(func['is_const']).lower()},\n")
                f.write(f"                retType,\n")
                f.write(f"                paramTypes,\n")
                f.write(f"                &{class_name}_{func['name']}_Invoke\n")
                f.write("            };\n")
                f.write(f"            {class_name}_Functions.push_back(std::move(rf));\n")
                f.write("        }\n")
            f.write(f"        ci.functions = {class_name}_Functions;\n")

        # Variables
        if cls["variables"]:
            f.write(f"        {class_name}_Variables.clear();\n")
            for var in cls["variables"]:
                f.write(
                    f'        {"auto*" if funcCounter == 0 else ""} vType = Reflection::Registry::Instance().GetOrCreateType("{var["type"]}");\n')
                f.write("        {\n")
                f.write(f"            Reflection::ReflectedVariable rv = {{\n")
                f.write(f'                "{var["name"]}", "{var["access"]}",\n')
                f.write(f"                {str(var['is_static']).lower()},\n")
                f.write(f"                offsetof({class_name}, {var['name']}),\n")
                f.write("                vType\n")
                f.write("            };\n")
                f.write(f"            {class_name}_Variables.push_back(std::move(rv));\n")
                f.write("        }\n")
                funcCounter += 1
            f.write(f"        ci.variables = {class_name}_Variables;\n")

        # <--- MODIFIED: Call the correct registration function based on the stored type
        reflection_type = cls.get("reflection_type", "Class")
        if reflection_type == "Component":
            f.write("        Reflection::Registry::Instance().RegisterComponent(std::move(ci));\n")
        elif reflection_type == "System":
            f.write("        Reflection::Registry::Instance().RegisterSystem(std::move(ci));\n")
        else:  # Default to "Class"
            f.write("        Reflection::Registry::Instance().RegisterClass(std::move(ci));\n")

        f.write("    }\n")
        f.write("};\n")
        f.write(f"static {class_name}_AutoRegister _{class_name.lower()}_autoreg;\n\n")

    def _emit_function_invoker(self, f, class_name, func):
        # ... (this function remains the same) ...
        func_name = func["name"]
        params = func["params"]
        ret = func["return_type"]

        f.write(f"static void {class_name}_{func_name}_Invoke(void* instance, void** args, void* ret) {{\n")
        f.write(f"    if ({class_name}_{func_name}_Hook) {{\n")
        f.write(f"        {class_name}_{func_name}_Hook(instance, args, ret);\n")
        f.write("        return;\n")
        f.write("    }\n")
        if func["is_static"]:
            call_prefix = f"{class_name}::{func_name}("
        else:
            f.write(f"    auto* obj = static_cast<{class_name}*>(instance);\n")
            call_prefix = f"obj->{func_name}("
        arg_exprs = [f"*static_cast<{t}*>(args[{i}])" for i, (t, _) in enumerate(params)]
        call_suffix = ", ".join(arg_exprs) + ")"
        if ret != "void":
            f.write(f"    *static_cast<{ret}*>(ret) = {call_prefix}{call_suffix};\n")
        else:
            f.write(f"    {call_prefix}{call_suffix};\n")
        f.write("}\n\n")

    def generate_master_include(self, output_dir: str):
        # ... (this function remains the same) ...
        import networkx as nx

        cpp_files = [f for f in os.listdir(output_dir) if f.endswith(".gen.cpp")]
        file_to_classes = defaultdict(list)
        class_to_file = {}

        if not nx:
            sorted_files = sorted([f for f in cpp_files if f != "AllGenerated.cpp"])
        else:
            file_to_classes = defaultdict(list)
            class_to_file = {}
            G = nx.DiGraph()

            G.add_nodes_from(cpp_files)
            for fpath in cpp_files:
                full_path = os.path.join(output_dir, fpath)
                with open(full_path, "r", encoding="utf-8") as f:
                    content = f.read()
                import re
                matches = re.findall(r'ci\.name\s*=\s*"([^"]+)"', content)
                for cls_name in matches:
                    class_to_file[cls_name] = fpath
                    file_to_classes[fpath].append(cls_name)

            for fpath in cpp_files:
                with open(os.path.join(output_dir, fpath), "r", encoding="utf-8") as f:
                    content = f.read()
                base_names = re.findall(r'FindClass\("([^"]+)"\)', content)
                for base in base_names:
                    if base in class_to_file and class_to_file[base] != fpath:
                        G.add_edge(class_to_file[base], fpath)

            try:
                sorted_files = list(nx.topological_sort(G))
            except nx.NetworkXUnfeasible:
                print("[Warning] Cyclic dependency detected. Falling back to alphabetical order.")
                sorted_files = sorted(cpp_files)
        master_path = os.path.join(output_dir, "AllGenerated.cpp")
        with open(master_path, "w", encoding="utf-8") as f:
            f.write("// Auto-generated master reflection file\n")
            f.write('#include "ReflectionEngine.h"\n\n')
            for cpp_file in sorted_files:
                if cpp_file != "AllGenerated.cpp":
                    f.write(f'#include "{cpp_file}"\n')
            f.write("\nnamespace ReflectionSystem {\n")
            f.write("    // All classes auto-register via static constructors\n")
            f.write("}\n")
        print(f"[CodeGen] Generated master include: {master_path}")
        return master_path


# ... (main function remains the same) ...
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--scan-dir", required=True)
    parser.add_argument("--out-dir", required=True)

    parser.add_argument("--ms-includes", default="",
                        help="Semicolon-delimited string of include paths from MSBuild.")

    parser.add_argument("-v", "--verbose", action="store_true")
    parser.add_argument("--libclang", help="Path to libclang.dll or libclang.so")
    args = parser.parse_args()

    if args.libclang:
        clang.cindex.Config.set_library_file(args.libclang)

    scan_dir = Path(args.scan_dir)
    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    clang_parse_args = [
        "-std=c++17",
        "-x", "c++",
        "-I."
    ]
    if args.ms_includes:
        for path in args.ms_includes.split(';'):
            if path:
                clang_parse_args.append(f"-I{path}")

    print("[CodeGen] Paths:")
    print(args.ms_includes)
    print("[CodeGen] Using Clang arguments:")
    for arg in clang_parse_args:
        print(f"  {arg}")

    gen = ReflectionGenerator()
    header_files = list(scan_dir.rglob("*.h")) + list(scan_dir.rglob("*.hpp"))
    for file_path in header_files:
        if args.verbose:
            print("Parsing", file_path)
        gen.parse(str(file_path), clang_parse_args)

    if not gen.classes:
        print("No reflected classes found")
        return 0

    classes_by_file = defaultdict(list)
    for filepath, cls in gen.classes:
        classes_by_file[filepath].append(cls)

    for filepath, classes in classes_by_file.items():
        header_file = Path(filepath).name
        cpp_out = out_dir / f"{header_file}.gen.cpp"
        gen.generate_cpp_for_classes(str(cpp_out), header_file, classes)
        if args.verbose:
            print("Generated", cpp_out, "for", [c["name"] for c in classes])

    gen.generate_master_include(str(out_dir))
    print("Done.")
    return 0


if __name__ == "__main__":
    sys.exit(main())