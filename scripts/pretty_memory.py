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

def get_bin_dir() -> Path:
    return get_repo_dir() / "bin"

def get_build_dir() -> Path:
    return get_repo_dir() / "build"

def get_doc_dir() -> Path:
    return get_repo_dir() / "doc"

def get_include_dir() -> Path:
    return get_repo_dir() / "include"

def get_source_dir() -> Path:
    return get_repo_dir()

def get_solution_file() -> Path:
    return get_build_dir() / "PrettyMemory.slnx"


def path_to_string(path: Path) -> str:
    if sys.platform == "Windows":
        return str(path).replace('\\', '/')
    else:
        return str(path)

def run_command(args: Sequence[str]):
    print(f">> {' '.join(args)}")
    subprocess.run(args, check=True)


def get_vs_path() -> Path | None:
    value = os.getenv('VS_PATH')
    return Path(value) if value else None
