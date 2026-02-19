import tkinter as tk
from tkinter import filedialog, messagebox, ttk
import os
import shutil
import uuid
import subprocess
import json
from jinja2 import Environment, FileSystemLoader

ENGINE_PATH = r"D:\test\Inzynierka\ReEngine\bin\Debug"
ENGINE_SOURCE_PATH = r"D:\test\Inzynierka\ReEngine"
EDITOR_PATH = r"D:\test\Inzynierka\ReEngine\bin\Debug\ReEngineEditor.exe"
CLANG_REFLECTION_SCRIPT = r"C:\Path\To\reflection_generator.py"

env = Environment(loader=FileSystemLoader("templates"))


class ProjectCreator:
    def __init__(self):
        self.setup_gui()

    def setup_gui(self):
        self.root = tk.Tk()
        self.root.title("ReEngine Project Creator")
        self.root.geometry("600x700")

        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        config_frame = ttk.LabelFrame(main_frame, text="Project Configuration", padding="10")
        config_frame.grid(row=0, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=(0, 10))

        ttk.Label(config_frame, text="Project Name:").grid(row=0, column=0, sticky="w")
        self.project_name_var = tk.StringVar()
        self.project_name_entry = ttk.Entry(config_frame, textvariable=self.project_name_var, width=40)
        self.project_name_entry.grid(row=0, column=1, padx=(10, 0), pady=2)

        ttk.Label(config_frame, text="Project Directory:").grid(row=1, column=0, sticky="w")
        self.project_dir_var = tk.StringVar()
        self.project_dir_entry = ttk.Entry(config_frame, textvariable=self.project_dir_var, width=40)
        self.project_dir_entry.grid(row=1, column=1, padx=(10, 0), pady=2)
        ttk.Button(config_frame, text="Browse", command=self.browse_directory).grid(row=1, column=2, padx=(5, 0))

        ttk.Label(config_frame, text="Project Type:").grid(row=2, column=0, sticky="w")
        self.project_type_var = tk.StringVar(value="Game")
        project_type_combo = ttk.Combobox(config_frame, textvariable=self.project_type_var,
                                          values=["Game", "Plugin", "Tool"], state="readonly", width=37)
        project_type_combo.grid(row=2, column=1, padx=(10, 0), pady=2)

        paths_frame = ttk.LabelFrame(main_frame, text="Engine Paths", padding="10")
        paths_frame.grid(row=1, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=(0, 10))

        ttk.Label(paths_frame, text="Engine Path:").grid(row=0, column=0, sticky="w")
        self.engine_path_var = tk.StringVar(value=ENGINE_PATH)
        engine_path_entry = ttk.Entry(paths_frame, textvariable=self.engine_path_var, width=40)
        engine_path_entry.grid(row=0, column=1, padx=(10, 0), pady=2)
        ttk.Button(paths_frame, text="Browse", command=lambda: self.browse_file(self.engine_path_var, "folder")).grid(
            row=0, column=2, padx=(5, 0))

        ttk.Label(paths_frame, text="Engine Source Path:").grid(row=1, column=0, sticky="w")
        self.engine_source_path_var = tk.StringVar(value=ENGINE_SOURCE_PATH)
        engine_source_path_entry = ttk.Entry(paths_frame, textvariable=self.engine_source_path_var, width=40)
        engine_source_path_entry.grid(row=1, column=1, padx=(10, 0), pady=2)
        ttk.Button(paths_frame, text="Browse",
                   command=lambda: self.browse_file(self.engine_source_path_var, "folder")).grid(
            row=1, column=2, padx=(5, 0))

        ttk.Label(paths_frame, text="Editor Path:").grid(row=2, column=0, sticky="w")
        self.editor_path_var = tk.StringVar(value=EDITOR_PATH)
        editor_path_entry = ttk.Entry(paths_frame, textvariable=self.editor_path_var, width=40)
        editor_path_entry.grid(row=2, column=1, padx=(10, 0), pady=2)
        ttk.Button(paths_frame, text="Browse", command=lambda: self.browse_file(self.editor_path_var, "file")).grid(
            row=2, column=2, padx=(5, 0))

        button_frame = ttk.Frame(main_frame)
        button_frame.grid(row=3, column=0, columnspan=3, pady=10)

        ttk.Button(button_frame, text="Create Project",
                   command=self.create_project, style="Accent.TButton").pack(side=tk.LEFT, padx=5)
        ttk.Button(button_frame, text="Open in Visual Studio",
                   command=self.create_project).pack(side=tk.LEFT, padx=5)
        ttk.Button(button_frame, text="Launch Editor",
                   command=self.create_project).pack(side=tk.LEFT, padx=5)
        ttk.Button(button_frame, text="Build & Run",
                   command=self.create_project).pack(side=tk.LEFT, padx=5)

        log_frame = ttk.LabelFrame(main_frame, text="Status", padding="10")
        log_frame.grid(row=4, column=0, columnspan=3, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(10, 0))

        self.log_text = tk.Text(log_frame, height=8, width=70)
        scrollbar = ttk.Scrollbar(log_frame, orient="vertical", command=self.log_text.yview)
        self.log_text.configure(yscrollcommand=scrollbar.set)
        self.log_text.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S))

        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(1, weight=1)
        main_frame.rowconfigure(4, weight=1)
        log_frame.columnconfigure(0, weight=1)
        log_frame.rowconfigure(0, weight=1)

    def log(self, message):
        """Add message to log area"""
        self.log_text.insert(tk.END, f"{message}\n")
        self.log_text.see(tk.END)
        self.root.update_idletasks()

    def browse_directory(self):
        folder_selected = filedialog.askdirectory()
        if folder_selected:
            self.project_dir_var.set(folder_selected)

    def browse_file(self, var, file_type):
        if file_type == "folder":
            path = filedialog.askdirectory()
        else:
            path = filedialog.askopenfilename()
        if path:
            var.set(path)

    def create_project(self):
        project_name = self.project_name_var.get().strip()
        project_dir = self.project_dir_var.get().strip()

        if not project_name or not project_dir:
            messagebox.showerror("Error", "Please enter project name and directory")
            return

        try:
            self.log(f"Creating project '{project_name}'...")
            self.generate_project(project_name, project_dir)
            self.log("Project created successfully!")
        except Exception as e:
            self.log(f"Error creating project: {str(e)}")
            messagebox.showerror("Error", f"Failed to create project: {str(e)}")

    def generate_project(self, project_name, project_dir):
        project_guid = "{" + str(uuid.uuid4()).upper() + "}"
        sln_guid = "{" + str(uuid.uuid4()).upper() + "}"
        project_root = os.path.join(project_dir, project_name)

        self.log("Creating directory structure...")

        directories = [
            "Source",
            "Generated",
            "ThirdParty/include",
            "ThirdParty/lib",
            "Content/Assets",
            "Content/Scripts",
            "Build/Scripts",
            "Intermediate",
            "shaders",
            "icons",
            "bin/Debug"
        ]

        for dir_path in directories:
            os.makedirs(os.path.join(project_root, dir_path), exist_ok=True)

        self.log("Creating AllGenerated.cpp...")
        generated_file_path = os.path.join(project_root, "Generated", "AllGenerated.cpp")
        try:
            with open(generated_file_path, "w") as f:
                f.write("// This file is auto-generated by ReEngine Project Creator\n")
                f.write("// It serves as a compilation unit for generated reflection data.\n\n")
                f.write("#include <iostream>\n")
                f.write("// #include \"Generated/ReflectionRegistry.h\" \n")
        except Exception as e:
            self.log(f"Error creating AllGenerated.cpp: {e}")

        self.log("Copying shaders...")
        script_dir = os.path.dirname(os.path.abspath(__file__))
        source_shaders_path = os.path.join(script_dir, "shaders")
        dest_shaders_path = os.path.join(project_root, "shaders")

        if os.path.exists(source_shaders_path):
            try:

                if os.path.exists(dest_shaders_path):
                    shutil.rmtree(dest_shaders_path)

                shutil.copytree(source_shaders_path, dest_shaders_path)
                self.log(f"Shaders copied from {source_shaders_path}")
            except Exception as e:
                self.log(f"Error copying shaders: {e}")
        else:
            self.log(f"Warning: 'shaders' folder not found at {source_shaders_path}. Skipping copy.")


        self.log("Copying icons...")
        script_dir = os.path.dirname(os.path.abspath(__file__))
        source_icons_path = os.path.join(script_dir, "icons")
        dest_icons_path = os.path.join(project_root, "icons")

        if os.path.exists(source_icons_path):
            try:

                if os.path.exists(dest_icons_path):
                    shutil.rmtree(dest_icons_path)

                shutil.copytree(source_icons_path, dest_icons_path)
                self.log(f"Icons copied from {source_icons_path}")
            except Exception as e:
                self.log(f"Error copying icons: {e}")
        else:
            self.log(f"Warning: 'icons' folder not found at {source_icons_path}. Skipping copy.")

        self.log("Copying imgui.ini...")
        source_imgui_path = os.path.join(script_dir, "imgui.ini")
        dest_imgui_path = os.path.join(project_root, "bin", "Debug", "imgui.ini")

        if os.path.exists(source_imgui_path):
            try:
                shutil.copy2(source_imgui_path, dest_imgui_path)
                self.log(f"imgui.ini copied to {dest_imgui_path}")
            except Exception as e:
                self.log(f"Error copying imgui.ini: {e}")
        else:
            self.log(f"Warning: 'imgui.ini' not found at {source_imgui_path}. Skipping copy.")


        self.log("Creating project configuration...")
        project_config = {
            "name": project_name,
            "type": self.project_type_var.get(),
            "engine_path": self.editor_path_var.get(),
            "editor_path": self.editor_path_var.get(),
            "engine_source_path": self.engine_source_path_var.get(),
            "version": "1.0.0",
            "": "",
            "Renderer": "OpenGlRenderer",
            "Physics": "Physics3D"
        }

        self.log(str(project_config))

        with open(os.path.join(project_root, f"{project_name}.json"), "w") as f:
            json.dump(project_config, f, indent=2)

        self.log("Generating Visual Studio project files...")
        self.generate_vs_project_files(project_root, project_name, project_guid, sln_guid)

        self.log(f"Project '{project_name}' created successfully at: {project_root}")

    def generate_vs_project_files(self, project_root, project_name, project_guid, sln_guid):
        try:
            # Generate .vcxproj file
            vcxproj_template = env.get_template("vcxproj.jinja")
            with open(os.path.join(project_root, f"{project_name}.vcxproj"), "w") as f:
                f.write(vcxproj_template.render(
                    project_name=project_name,
                    project_guid=project_guid,
                    project_type=self.project_type_var.get(),
                    engine_path=ENGINE_PATH,
                    engine_source_path=self.engine_source_path_var.get(),
                    editor_path=self.editor_path_var.get()
                ))

            sln_template = env.get_template("sln.jinja")
            with open(os.path.join(project_root, f"{project_name}.sln"), "w") as f:
                f.write(sln_template.render(
                    project_name=project_name,
                    project_guid=project_guid,
                    sln_guid=sln_guid
                ))

            self.generate_props_file(project_root, project_name)
        except Exception as e:
            self.log(f"Warning: Could not generate VS project files: {e}")

    def generate_props_file(self, project_root, project_name):
        props_content = f'''<?xml version="1.0" encoding="utf-8"?>
    <Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
      <ImportGroup Label="PropertySheets" />
      <PropertyGroup Label="UserMacros" />
      <PropertyGroup>
        <IncludePath>$(ProjectDir)ThirdParty\\include;$(IncludePath)</IncludePath>
        <LibraryPath>$(ProjectDir)ThirdParty\\lib;$(LibraryPath)</LibraryPath>
        <OutDir>$(ProjectDir)Build\\Binaries\\$(Configuration)\\</OutDir>
        <IntDir>$(ProjectDir)Intermediate\\$(Configuration)\\</IntDir>
      </PropertyGroup>
      <ItemDefinitionGroup>
        <ClCompile>
          <PreprocessorDefinitions>REFLECTION_ENABLED;%(PreprocessorDefinitions)</PreprocessorDefinitions>
        </ClCompile>
      </ItemDefinitionGroup>
      <ItemGroup />
      <ImportGroup Label="ExtensionTargets" />
    </Project>'''

        with open(os.path.join(project_root, f"{project_name}.props"), "w") as f:
            f.write(props_content)

    def open_in_visual_studio(self):
        project_name = self.project_name_var.get().strip()
        project_dir = self.project_dir_var.get().strip()

        if not project_name or not project_dir:
            messagebox.showerror("Error", "Please enter project name and directory")
            return

        sln_path = os.path.join(project_dir, project_name, f"{project_name}.sln")
        if os.path.exists(sln_path):
            try:
                subprocess.Popen(["devenv", sln_path])
                self.log(f"Opening {project_name} in Visual Studio...")
            except Exception as e:
                self.log(f"Failed to open Visual Studio: {e}")
                messagebox.showerror("Error", f"Failed to open Visual Studio: {e}")
        else:
            messagebox.showerror("Error", f"Solution file not found: {sln_path}")

    def run(self):
        self.root.mainloop()


if __name__ == "__main__":
    app = ProjectCreator()
    app.run()