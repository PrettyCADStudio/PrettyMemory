# coding: utf-8

import sys
import subprocess

from pretty_memory import get_solution_file, get_vs_path, path_to_string, get_cmake_file


def main_windows():
    vs_path = get_vs_path()
    if vs_path is None:
        print(f"Visual Studio path was not found")
        return

    solution_file = get_solution_file()
    if not solution_file.is_file():
        print(f"Solution file was not found: {solution_file}")
        return

    cmd = [
        path_to_string(vs_path),
        path_to_string(solution_file)
    ]
    print(f">> {' '.join(cmd)}")
    subprocess.Popen(cmd)


def main_linux():
    cmake_file = get_cmake_file()
    cmd = [
        'clion',
        path_to_string(cmake_file)
    ]
    subprocess.Popen(cmd)


if __name__ == "__main__":
    if sys.platform == "win32":
        main_windows()
    else:
        main_linux()
