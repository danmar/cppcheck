#!/usr/bin/env python3
#
# Library for build system detection and compile_commands.json generation.
#
# This module provides functions to detect build systems in project directories
# and generate compile_commands.json files. It is designed to be imported and
# used by other scripts, particularly the y2038 addon.
#

import os
import shutil
import subprocess
import sys
from pathlib import Path

def print_error(message):
    """Prints an error message to stderr."""
    print(f"Error: {message}", file=sys.stderr)

def run_command(cmd, cwd):
    """Runs a command in a specified directory and handles errors."""
    print(f"Running command: {' '.join(cmd)} in {cwd}")
    try:
        result = subprocess.run(cmd, cwd=cwd, check=True, capture_output=True, text=True)
        print(result.stdout)
        if result.stderr:
            print(result.stderr, file=sys.stderr)
        return True
    except FileNotFoundError:
        print_error(f"Command '{cmd[0]}' not found. Please ensure it is in your PATH.")
        return False
    except subprocess.CalledProcessError as e:
        print_error(f"Command failed with exit code {e.returncode}")
        print(e.stdout)
        print(e.stderr, file=sys.stderr)
        return False

def detect_build_system(project_dir):
    """
    Detects the build system by looking for characteristic files.
    The order of checks determines the precedence.
    """
    project_path = Path(project_dir)
    if (project_path / "CMakeLists.txt").exists():
        return "cmake"
    if (project_path / "meson.build").exists():
        return "meson"
    if (project_path / "configure").exists():
        return "autotools"
    if (project_path / "Makefile").exists():
        return "make"
    if (project_path / "BUILD.bazel").exists() or (project_path / "BUILD").exists():
        return "bazel"
    if (project_path / "Cargo.toml").exists():
        return "cargo"
    return None

def generate_for_cmake(project_dir):
    """Generates compile_commands.json for CMake projects."""
    print("CMake project detected.")
    build_dir = project_dir / "build"
    build_dir.mkdir(exist_ok=True)

    cmd = ["cmake", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", ".."]
    if not run_command(cmd, cwd=build_dir):
        return False

    generated_file = build_dir / "compile_commands.json"
    if generated_file.exists():
        shutil.copy(generated_file, project_dir)
        print(f"Successfully generated and copied compile_commands.json to {project_dir}")
        return True
    else:
        print_error("CMake did not generate compile_commands.json.")
        return False

def generate_for_meson(project_dir):
    """Generates compile_commands.json for Meson projects."""
    print("Meson project detected.")
    build_dir = project_dir / "build"
    
    # Meson requires the build directory not to exist for setup
    if build_dir.exists():
        print(f"Build directory {build_dir} already exists. Meson setup will not be run again.")
    else:
        cmd_setup = ["meson", "setup", "build"]
        if not run_command(cmd_setup, cwd=project_dir):
            return False

    generated_file = build_dir / "compile_commands.json"
    if generated_file.exists():
        shutil.copy(generated_file, project_dir)
        print(f"Successfully generated and copied compile_commands.json to {project_dir}")
        return True
    else:
        print_error("Meson did not generate compile_commands.json.")
        return False

def generate_for_make_autotools(project_dir, is_autotools):
    """Generates compile_commands.json for Make/Autotools projects using bear."""
    if is_autotools:
        print("Autotools project detected.")
    else:
        print("Make project detected.")

    if not shutil.which("bear"):
        print_error("'bear' is required for Make/Autotools projects but is not found in your PATH.")
        print("Please install it (e.g., 'sudo apt-get install bear' or 'brew install bear') and try again.")
        return False

    if is_autotools:
        if not run_command(["./configure"], cwd=project_dir):
            print_error("'./configure' script failed. Cannot proceed with make.")
            return False

    if not run_command(["bear", "--", "make"], cwd=project_dir):
        return False

    generated_file = project_dir / "compile_commands.json"
    if generated_file.exists():
        print(f"Successfully generated compile_commands.json in {project_dir}")
        return True
    else:
        print_error("bear did not generate compile_commands.json.")
        return False

def generate_compile_commands(project_dir):
    """
    Generate compile_commands.json for the given project directory.
    
    This function detects the build system and generates compile_commands.json
    using the appropriate method for the detected build system.
    
    Args:
        project_dir (Path): Path to the project directory
        
    Returns:
        bool: True if generation was successful, False otherwise
    """
    project_dir = Path(project_dir).resolve()
    if not project_dir.is_dir():
        print_error(f"Project directory not found: {project_dir}")
        return False

    print(f"Analyzing project in: {project_dir}")

    build_system = detect_build_system(project_dir)

    generated_successfully = False
    if build_system == "cmake":
        generated_successfully = generate_for_cmake(project_dir)
    elif build_system == "meson":
        generated_successfully = generate_for_meson(project_dir)
    elif build_system == "autotools":
        generated_successfully = generate_for_make_autotools(project_dir, is_autotools=True)
    elif build_system == "make":
        generated_successfully = generate_for_make_autotools(project_dir, is_autotools=False)
    elif build_system in ["bazel", "cargo"]:
        print(f"{build_system.capitalize()} project detected. Automatic generation of 'compile_commands.json' is not supported.")
        print(f"Please consult the {build_system.capitalize()} documentation to generate it manually.")
        return False
    else:
        print_error("Could not detect a supported build system (CMake, Meson, Make, Autotools).")
        return False

    if not generated_successfully:
        print_error("Failed to generate compile_commands.json.")
        return False

    compile_commands_path = project_dir / "compile_commands.json"
    if not compile_commands_path.exists():
        print_error(f"compile_commands.json not found at {compile_commands_path} after generation step.")
        return False
        
    return True


def main():
    """Main function for standalone script usage (deprecated - use as library instead)."""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Generate a compile_commands.json file for various build systems."
    )
    parser.add_argument(
        "project_directory",
        nargs="?",
        default=".",
        help="The path to the project's root directory (default: current directory)."
    )
    args = parser.parse_args()

    project_dir = Path(args.project_directory).resolve()
    
    success = generate_compile_commands(project_dir)
    if not success:
        sys.exit(1)
        
    print(f"Successfully generated compile_commands.json in {project_dir}")


if __name__ == "__main__":
    main()
