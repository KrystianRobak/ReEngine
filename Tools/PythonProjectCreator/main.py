import tkinter as tk
from tkinter import filedialog, messagebox, ttk
import os
import shutil
import uuid
import subprocess
import json
from jinja2 import Environment, FileSystemLoader

# Configuration - these should be set to your actual paths
ENGINE_PATH = r"C:\Users\ragbe\Desktop\Inzynierka\ReEngine\bin\Debug"
ENGINE_SOURCE_PATH = r"C:\Users\ragbe\Desktop\Inzynierka\ReEngine"
EDITOR_PATH = r"C:\Users\ragbe\Desktop\Inzynierka\ReEngine\bin\Debug\ReEngineEditor.exe"
CLANG_REFLECTION_SCRIPT = r"C:\Path\To\reflection_generator.py"
# Template environment
env = Environment(loader=FileSystemLoader("templates"))


class ProjectCreator:
    def __init__(self):
        self.setup_gui()

    def setup_gui(self):
        self.root = tk.Tk()
        self.root.title("ReEngine Project Creator")
        self.root.geometry("600x700")

        # Main frame
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        # Project configuration section
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

        # Project type
        ttk.Label(config_frame, text="Project Type:").grid(row=2, column=0, sticky="w")
        self.project_type_var = tk.StringVar(value="Game")
        project_type_combo = ttk.Combobox(config_frame, textvariable=self.project_type_var,
                                          values=["Game", "Plugin", "Tool"], state="readonly", width=37)
        project_type_combo.grid(row=2, column=1, padx=(10, 0), pady=2)

        # Engine paths section
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
        ttk.Button(paths_frame, text="Browse", command=lambda: self.browse_file(self.engine_source_path_var, "folder")).grid(
            row=1, column=2, padx=(5, 0))

        ttk.Label(paths_frame, text="Editor Path:").grid(row=2, column=0, sticky="w")
        self.editor_path_var = tk.StringVar(value=EDITOR_PATH)
        editor_path_entry = ttk.Entry(paths_frame, textvariable=self.editor_path_var, width=40)
        editor_path_entry.grid(row=2, column=1, padx=(10, 0), pady=2)
        ttk.Button(paths_frame, text="Browse", command=lambda: self.browse_file(self.editor_path_var, "file")).grid(
            row=2, column=2, padx=(5, 0))

        # Action buttons
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

        # Status/Log area
        log_frame = ttk.LabelFrame(main_frame, text="Status", padding="10")
        log_frame.grid(row=4, column=0, columnspan=3, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(10, 0))

        self.log_text = tk.Text(log_frame, height=8, width=70)
        scrollbar = ttk.Scrollbar(log_frame, orient="vertical", command=self.log_text.yview)
        self.log_text.configure(yscrollcommand=scrollbar.set)
        self.log_text.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S))

        # Configure grid weights
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

        # Create directory structure
        directories = [
            "Source",
            "Generated",
            "ThirdParty/include",
            "ThirdParty/lib",
            "Content/Assets",
            "Content/Scripts",
            "Build/Scripts",
            "Intermediate"
        ]

        for dir_path in directories:
            os.makedirs(os.path.join(project_root, dir_path), exist_ok=True)

        # Create project configuration file
        self.log("Creating project configuration...")
        project_config = {
            "name": project_name,
            "type": self.project_type_var.get(),
            "engine_path": self.editor_path_var.get(),
            "editor_path": self.editor_path_var.get(),
            "engine_source_path": self.engine_source_path_var.get(),
            "version": "1.0.0",
            "":"",
            "Renderer" : "OpenGlRenderer",
            "Physics" : "Physics3D"

        }

        self.log(project_config)

        with open(os.path.join(project_root, f"{project_name}.json"), "w") as f:
            json.dump(project_config, f, indent=2)


        # Generate Visual Studio project files
        self.log("Generating Visual Studio project files...")
        self.generate_vs_project_files(project_root, project_name, project_guid, sln_guid)

        self.log(f"Project '{project_name}' created successfully at: {project_root}")

    def generate_vs_project_files(self, project_root, project_name, project_guid, sln_guid):
        """Generate Visual Studio project and solution files"""
        try:
            # Generate .vcxproj file
            vcxproj_template = env.get_template("vcxproj.jinja")
            with open(os.path.join(project_root, f"{project_name}.vcxproj"), "w") as f:
                f.write(vcxproj_template.render(
                    project_name=project_name,
                    project_guid=project_guid,
                    project_type=self.project_type_var.get(),
                    engine_path=ENGINE_PATH,
                    engine_source_path = self.engine_source_path_var.get(),  # <-- add this
                    editor_path = self.editor_path_var.get()
                ))

            # Generate .sln file
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
            # Create basic files as fallback
            self.create_fallback_project_files(project_root, project_name, project_guid, sln_guid)


    def generate_props_file(self, project_root, project_name):
        """Generate MSBuild .props file for project configuration"""
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