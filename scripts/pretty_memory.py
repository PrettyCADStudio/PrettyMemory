# coding: utf-8

import os
import sys
import subprocess
from pathlib import Path
from typing import Sequence


def get_scripts_dir() -> Path:
    return Path(__file__).parent

def get_repo_dir() -> Path:
    return get_scripts_dir().parent

def get_bin_dir(create_if_not_exist: bool = True) -> Path:
    bin_dir = get_repo_dir() / "bin"
    if create_if_not_exist and not bin_dir.is_dir():
        bin_dir.mkdir(parents=True, exist_ok=True)
    return bin_dir

def get_build_dir(create_if_not_exist: bool = True) -> Path:
    build_dir = get_repo_dir() / "build"
    if create_if_not_exist and not build_dir.is_dir():
        build_dir.mkdir(parents=True, exist_ok=True)
    return build_dir

def get_doc_dir() -> Path:
    return get_repo_dir() / "doc"

def get_include_dir() -> Path:
    return get_repo_dir() / "include"

def get_source_dir() -> Path:
    return get_repo_dir()

def get_cmake_file() -> Path:
    return get_repo_dir() / "CMakeLists.txt"

def get_solution_file() -> Path:
    return get_build_dir() / "PrettyMemory.slnx"


def get_cmake_generator() -> str:
    if sys.platform == "win32":
        return "Visual Studio 18 2026"
    else:
        return "Unix Makefiles"

def path_to_string(path: Path) -> str:
    if sys.platform == "win32":
        return str(path).replace('\\', '/')
    else:
        return str(path)

def run_command(args: Sequence[str]):
    print(f">> {' '.join(args)}")
    subprocess.run(args, check=True)


def get_vs_path() -> Path | None:
    value = os.getenv('VS_PATH')
    return Path(value) if value else None
