# coding: utf-8

import subprocess

from pretty_memory import get_solution_file, get_vs_path, path_to_string


def main():
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


if __name__ == "__main__":
    main()
