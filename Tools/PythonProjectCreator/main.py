import tkinter as tk
from tkinter import filedialog, messagebox, ttk
import os
import shutil
import uuid
import subprocess
import json
from jinja2 import Environment, FileSystemLoader

# Configuration - these should be set to your actual paths
ENGINE_PATH = "C:/Users/ragberr/Desktop/ReEngine/x64/Debug"
EDITOR_PATH = "C:/Path/To/Editor.exe"
CLANG_REFLECTION_SCRIPT = "C:/Path/To/reflection_generator.py"  # Your Python reflection script

# Template environment
env = Environment(loader=FileSystemLoader("templates"))


class ProjectCreator:
    def __init__(self):
        self.setup_gui()

    def setup_gui(self):
        self.root = tk.Tk()
        self.root.title("ReEngine Project Creator")
        self.root.geometry("600x500")

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

        ttk.Label(paths_frame, text="Editor Path:").grid(row=1, column=0, sticky="w")
        self.editor_path_var = tk.StringVar(value=EDITOR_PATH)
        editor_path_entry = ttk.Entry(paths_frame, textvariable=self.editor_path_var, width=40)
        editor_path_entry.grid(row=1, column=1, padx=(10, 0), pady=2)
        ttk.Button(paths_frame, text="Browse", command=lambda: self.browse_file(self.editor_path_var, "file")).grid(
            row=1, column=2, padx=(5, 0))

        # Features section
        features_frame = ttk.LabelFrame(main_frame, text="Project Features", padding="10")
        features_frame.grid(row=2, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=(0, 10))

        self.enable_reflection_var = tk.BooleanVar(value=True)
        ttk.Checkbutton(features_frame, text="Enable Reflection System",
                        variable=self.enable_reflection_var).grid(row=0, column=0, sticky="w")

        self.create_sample_code_var = tk.BooleanVar(value=True)
        ttk.Checkbutton(features_frame, text="Create Sample Code",
                        variable=self.create_sample_code_var).grid(row=1, column=0, sticky="w")

        self.setup_build_scripts_var = tk.BooleanVar(value=True)
        ttk.Checkbutton(features_frame, text="Setup Build Scripts",
                        variable=self.setup_build_scripts_var).grid(row=2, column=0, sticky="w")

        # Action buttons
        button_frame = ttk.Frame(main_frame)
        button_frame.grid(row=3, column=0, columnspan=3, pady=10)

        ttk.Button(button_frame, text="Create Project",
                   command=self.create_project, style="Accent.TButton").pack(side=tk.LEFT, padx=5)
        ttk.Button(button_frame, text="Open in Visual Studio",
                   command=self.open_in_visual_studio).pack(side=tk.LEFT, padx=5)
        ttk.Button(button_frame, text="Launch Editor",
                   command=self.launch_editor).pack(side=tk.LEFT, padx=5)
        ttk.Button(button_frame, text="Build & Run",
                   command=self.build_and_run).pack(side=tk.LEFT, padx=5)

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

        # Copy engine dependencies
        self.log("Copying engine dependencies...")
        engine_path = self.engine_path_var.get()
        if os.path.exists(os.path.join(engine_path, "ReEngine.dll")):
            shutil.copy(os.path.join(engine_path, "ReEngine.dll"),
                        os.path.join(project_root, "ThirdParty"))

        # Copy engine headers if they exist
        engine_include_path = os.path.join(engine_path, "include")
        if os.path.exists(engine_include_path):
            shutil.copytree(engine_include_path,
                            os.path.join(project_root, "ThirdParty/include/Engine"),
                            dirs_exist_ok=True)

        # Create project configuration file
        self.log("Creating project configuration...")
        project_config = {
            "name": project_name,
            "type": self.project_type_var.get(),
            "engine_path": engine_path,
            "editor_path": self.editor_path_var.get(),
            "reflection_enabled": self.enable_reflection_var.get(),
            "version": "1.0.0"
        }

        with open(os.path.join(project_root, f"{project_name}.json"), "w") as f:
            json.dump(project_config, f, indent=2)

        # Generate source files
        self.log("Generating source files...")
        if self.create_sample_code_var.get():
            self.create_sample_source_files(project_root, project_name)

        # Generate reflection stub or run actual reflection
        self.log("Setting up reflection system...")
        if self.enable_reflection_var.get():
            self.setup_reflection(project_root, project_name)

        # Generate Visual Studio project files
        self.log("Generating Visual Studio project files...")
        self.generate_vs_project_files(project_root, project_name, project_guid, sln_guid)

        # Create build scripts
        if self.setup_build_scripts_var.get():
            self.log("Creating build scripts...")
            self.create_build_scripts(project_root, project_name)

        self.log(f"Project '{project_name}' created successfully at: {project_root}")

    def create_sample_source_files(self, project_root, project_name):
        """Create sample source files based on project type"""
        project_type = self.project_type_var.get()

        if project_type == "Game":
            # Create main game class
            main_cpp_content = f'''#include <iostream>
#include <memory>
#include "Engine/Core/Application.h"
#include "Engine/Core/Logger.h"
#include "{project_name}Application.h"

int main() {{
    try {{
        auto app = std::make_unique<{project_name}Application>();
        app->Initialize();
        app->Run();
        app->Shutdown();
    }}
    catch (const std::exception& e) {{
        ENGINE_ERROR("Application failed: {{}}", e.what());
        return -1;
    }}

    return 0;
}}'''

            app_header_content = f'''#pragma once
#include "Engine/Core/Application.h"

class {project_name}Application : public Engine::Application {{
public:
    {project_name}Application();
    virtual ~{project_name}Application();

    virtual bool Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual void Shutdown() override;

private:
    // Add your game-specific members here
}};'''

            app_cpp_content = f'''#include "{project_name}Application.h"
#include "Engine/Core/Logger.h"

{project_name}Application::{project_name}Application() {{
    ENGINE_INFO("Creating {project_name} application");
}}

{project_name}Application::~{project_name}Application() {{
    ENGINE_INFO("Destroying {project_name} application");
}}

bool {project_name}Application::Initialize() {{
    ENGINE_INFO("Initializing {project_name}");

    // Initialize your game systems here

    return true;
}}

void {project_name}Application::Update(float deltaTime) {{
    // Update your game logic here
}}

void {project_name}Application::Render() {{
    // Render your game here
}}

void {project_name}Application::Shutdown() {{
    ENGINE_INFO("Shutting down {project_name}");

    // Cleanup your game systems here
}}'''

            # Write files
            with open(os.path.join(project_root, "Source", f"{project_name}.cpp"), "w") as f:
                f.write(main_cpp_content)

            with open(os.path.join(project_root, "Source", f"{project_name}Application.h"), "w") as f:
                f.write(app_header_content)

            with open(os.path.join(project_root, "Source", f"{project_name}Application.cpp"), "w") as f:
                f.write(app_cpp_content)

    def setup_reflection(self, project_root, project_name):
        """Setup reflection system"""
        reflection_stub = '''// Auto-generated reflection code
// This file is generated by the reflection system
#pragma once

#include "Engine/Reflection/ReflectionMacros.h"

// Reflection data will be generated here by the build process
'''

        with open(os.path.join(project_root, "Generated", "ReflectionData.h"), "w") as f:
            f.write(reflection_stub)

        # Create reflection config file
        reflection_config = {
            "source_directories": ["Source"],
            "output_directory": "Generated",
            "include_patterns": ["*.h", "*.hpp"],
            "reflection_macros": ["REFLECT_CLASS", "REFLECT_PROPERTY", "REFLECT_METHOD"]
        }

        with open(os.path.join(project_root, "reflection_config.json"), "w") as f:
            json.dump(reflection_config, f, indent=2)

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
                    engine_path=ENGINE_PATH
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


    def create_build_scripts(self, project_root, project_name):
        """Create build scripts for the project"""

        # Windows batch script
        build_bat_content = f'''@echo off
echo Building {project_name}...

REM Run reflection generation if enabled
if exist "reflection_config.json" (
    echo Running reflection generation...
    python "{CLANG_REFLECTION_SCRIPT}" --config reflection_config.json
)

REM Build the project
echo Building Visual Studio project...
msbuild {project_name}.sln /p:Configuration=Debug /p:Platform=x64

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
) else (
    echo Build failed!
    pause
    exit /b %ERRORLEVEL%
)

pause'''

        # PowerShell script
        build_ps1_content = f'''# Build script for {project_name}
Write-Host "Building {project_name}..." -ForegroundColor Green

# Run reflection generation if enabled
if (Test-Path "reflection_config.json") {{
    Write-Host "Running reflection generation..." -ForegroundColor Yellow
    python "{CLANG_REFLECTION_SCRIPT}" --config reflection_config.json
    if ($LASTEXITCODE -ne 0) {{
        Write-Host "Reflection generation failed!" -ForegroundColor Red
        exit $LASTEXITCODE
    }}
}}

# Build the project
Write-Host "Building Visual Studio project..." -ForegroundColor Yellow
msbuild {project_name}.sln /p:Configuration=Debug /p:Platform=x64 /v:minimal

if ($LASTEXITCODE -eq 0) {{
    Write-Host "Build successful!" -ForegroundColor Green
}} else {{
    Write-Host "Build failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}}'''

        # Write build scripts
        with open(os.path.join(project_root, "Build/Scripts", "build.bat"), "w") as f:
            f.write(build_bat_content)

        with open(os.path.join(project_root, "Build/Scripts", "build.ps1"), "w") as f:
            f.write(build_ps1_content)

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

    def launch_editor(self):
        project_name = self.project_name_var.get().strip()
        project_dir = self.project_dir_var.get().strip()
        editor_path = self.editor_path_var.get().strip()

        if not project_name or not project_dir:
            messagebox.showerror("Error", "Please enter project name and directory")
            return

        if not os.path.exists(editor_path):
            messagebox.showerror("Error", f"Editor not found: {editor_path}")
            return

        project_path = os.path.join(project_dir, project_name)
        try:
            subprocess.Popen([editor_path, f"--project-path={project_path}"])
            self.log(f"Launching editor with project: {project_name}")
        except Exception as e:
            self.log(f"Failed to launch editor: {e}")
            messagebox.showerror("Error", f"Failed to launch editor: {e}")

    def build_and_run(self):
        """Build the project and run it"""
        project_name = self.project_name_var.get().strip()
        project_dir = self.project_dir_var.get().strip()

        if not project_name or not project_dir:
            messagebox.showerror("Error", "Please enter project name and directory")
            return

        project_path = os.path.join(project_dir, project_name)
        build_script = os.path.join(project_path, "Build/Scripts", "build.bat")

        if os.path.exists(build_script):
            try:
                self.log("Running build script...")
                subprocess.run([build_script], cwd=project_path, check=True)
                self.log("Build completed successfully!")
            except subprocess.CalledProcessError as e:
                self.log(f"Build failed with error code: {e.returncode}")
                messagebox.showerror("Build Error", f"Build failed with error code: {e.returncode}")
        else:
            self.log("Build script not found, attempting direct MSBuild...")
            sln_path = os.path.join(project_path, f"{project_name}.sln")
            if os.path.exists(sln_path):
                try:
                    subprocess.run([
                        "msbuild", sln_path,
                        "/p:Configuration=Debug",
                        "/p:Platform=x64"
                    ], check=True)
                    self.log("Build completed successfully!")
                except subprocess.CalledProcessError as e:
                    self.log(f"Build failed: {e}")
                    messagebox.showerror("Build Error", f"Build failed: {e}")

    def run(self):
        self.root.mainloop()


if __name__ == "__main__":
    app = ProjectCreator()
    app.run()